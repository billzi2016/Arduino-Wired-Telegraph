#include "protocol.h"

#include <string.h>

#include "morse.h"
#include "telegraph_config.h"

namespace {

constexpr char HEX_DIGITS[] = "0123456789ABCDEF";

bool isHexChar(char ch) {
  return (ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F');
}

}  // namespace

TelegraphProtocol::TelegraphProtocol(SingleWireBus &bus) : _bus(bus) {}

void TelegraphProtocol::begin() {
  randomSeed(analogRead(A0) + analogRead(A1));
  _currentSeq = static_cast<uint8_t>(random(0, 256));

  Serial.println(F("==== Arduino Wired Telegraph ===="));
  Serial.println(F("两块板子烧同一份程序。"));
  Serial.println(F("串口输入一行文本，按回车后自动发送。"));
  Serial.println(F("发送时 D13 蜂鸣器响，接收时 D12 亮。"));
  Serial.println(F("支持单线半双工、退避仲裁、ACK 和超时重发。"));
  Serial.println();
}

void TelegraphProtocol::tick() {
  processIncomingFrames();

  const unsigned long now = millis();

  switch (_state) {
    case NodeState::IDLE:
      startArbitrationIfNeeded();
      break;

    case NodeState::WAIT_IDLE:
      handleWaitIdleState(now);
      break;

    case NodeState::BACKOFF:
      handleBackoffState(now);
      break;

    case NodeState::WAIT_ACK:
      handleWaitAckState(now);
      break;
  }
}

bool TelegraphProtocol::enqueueUserMessage(const char *rawLine) {
  char sanitized[MAX_PAYLOAD_CHARS + 1] = {0};
  if (!sanitizeMessage(rawLine, sanitized, sizeof(sanitized))) {
    Serial.println(F("输入为空，已忽略。"));
    return false;
  }

  if (!enqueueSanitized(sanitized)) {
    Serial.println(F("发送队列已满，当前消息未入队。"));
    return false;
  }

  Serial.print(F("已入队: "));
  Serial.println(sanitized);
  return true;
}

bool TelegraphProtocol::sanitizeMessage(const char *rawLine, char *out, size_t outSize) {
  if (rawLine == nullptr || out == nullptr || outSize == 0) {
    return false;
  }

  size_t writeIndex = 0;
  bool lastWasSpace = true;

  for (size_t i = 0; rawLine[i] != '\0'; ++i) {
    char ch = rawLine[i];
    if (ch == '\r' || ch == '\n') {
      continue;
    }

    ch = morseNormalizeChar(ch);
    if (ch == ' ') {
      if (lastWasSpace) {
        continue;
      }
    }

    if (writeIndex + 1 >= outSize) {
      break;
    }

    out[writeIndex++] = ch;
    lastWasSpace = (ch == ' ');
  }

  while (writeIndex > 0 && out[writeIndex - 1] == ' ') {
    --writeIndex;
  }

  out[writeIndex] = '\0';
  return writeIndex > 0;
}

bool TelegraphProtocol::enqueueSanitized(const char *message) {
  if (queueFull()) {
    return false;
  }

  strncpy(_queue[_queueTail], message, MAX_PAYLOAD_CHARS);
  _queue[_queueTail][MAX_PAYLOAD_CHARS] = '\0';
  _queueTail = (_queueTail + 1) % MESSAGE_QUEUE_SIZE;
  ++_queueCount;
  return true;
}

bool TelegraphProtocol::buildDataFrame(uint8_t seq, const char *payload, char *out, size_t outSize) const {
  if (payload == nullptr || out == nullptr) {
    return false;
  }

  const size_t payloadLen = strlen(payload);
  const size_t totalLen = 1 + 2 + 2 + payloadLen + 2;
  if (totalLen + 1 > outSize || payloadLen > 0xFF) {
    return false;
  }

  out[0] = 'D';
  byteToHex(seq, out + 1);
  byteToHex(static_cast<uint8_t>(payloadLen), out + 3);
  memcpy(out + 5, payload, payloadLen);
  const uint8_t crc = computeFrameCrc(out, 5 + payloadLen);
  byteToHex(crc, out + 5 + payloadLen);
  out[totalLen] = '\0';
  return true;
}

bool TelegraphProtocol::buildAckFrame(uint8_t seq, char *out, size_t outSize) const {
  if (out == nullptr || outSize < 6) {
    return false;
  }

  out[0] = 'A';
  byteToHex(seq, out + 1);
  const uint8_t crc = computeFrameCrc(out, 3);
  byteToHex(crc, out + 3);
  out[5] = '\0';
  return true;
}

bool TelegraphProtocol::parseDataFrame(const char *frame, uint8_t &seq, char *payload, size_t payloadSize) const {
  if (frame == nullptr || payload == nullptr) {
    return false;
  }

  const size_t frameLen = strlen(frame);
  if (frameLen < 7 || frame[0] != 'D') {
    return false;
  }

  uint8_t parsedSeq = 0;
  uint8_t payloadLen = 0;
  uint8_t frameCrc = 0;

  if (!hexToByte(frame + 1, parsedSeq) || !hexToByte(frame + 3, payloadLen)) {
    return false;
  }

  if (frameLen != static_cast<size_t>(5 + payloadLen + 2)) {
    return false;
  }

  if (!hexToByte(frame + 5 + payloadLen, frameCrc)) {
    return false;
  }

  if (computeFrameCrc(frame, 5 + payloadLen) != frameCrc) {
    return false;
  }

  if (payloadLen + 1 > payloadSize) {
    return false;
  }

  memcpy(payload, frame + 5, payloadLen);
  payload[payloadLen] = '\0';
  seq = parsedSeq;
  return true;
}

bool TelegraphProtocol::parseAckFrame(const char *frame, uint8_t &seq) const {
  if (frame == nullptr || frame[0] != 'A' || strlen(frame) != 5) {
    return false;
  }

  uint8_t parsedSeq = 0;
  uint8_t frameCrc = 0;
  if (!hexToByte(frame + 1, parsedSeq) || !hexToByte(frame + 3, frameCrc)) {
    return false;
  }

  if (computeFrameCrc(frame, 3) != frameCrc) {
    return false;
  }

  seq = parsedSeq;
  return true;
}

uint8_t TelegraphProtocol::computeFrameCrc(const char *text, size_t length) const {
  uint8_t crc = 0;
  for (size_t i = 0; i < length; ++i) {
    crc ^= static_cast<uint8_t>(text[i]);
  }
  return crc;
}

void TelegraphProtocol::byteToHex(uint8_t value, char *out) const {
  out[0] = HEX_DIGITS[(value >> 4) & 0x0F];
  out[1] = HEX_DIGITS[value & 0x0F];
}

bool TelegraphProtocol::hexToByte(const char *text, uint8_t &value) const {
  if (!isHexChar(text[0]) || !isHexChar(text[1])) {
    return false;
  }

  auto hexValue = [](char ch) -> uint8_t {
    return (ch <= '9') ? static_cast<uint8_t>(ch - '0')
                       : static_cast<uint8_t>(ch - 'A' + 10);
  };

  value = static_cast<uint8_t>((hexValue(text[0]) << 4) | hexValue(text[1]));
  return true;
}

void TelegraphProtocol::processIncomingFrames() {
  char frame[MAX_FRAME_CHARS] = {0};
  while (_bus.popReceivedFrame(frame, sizeof(frame))) {
    handleIncomingFrame(frame);
  }
}

void TelegraphProtocol::handleIncomingFrame(const char *frame) {
  if (frame == nullptr || frame[0] == '\0') {
    return;
  }

  Serial.print(F("收到原始帧: "));
  Serial.println(frame);

  if (frame[0] == 'D') {
    handleIncomingData(frame);
    return;
  }

  if (frame[0] == 'A') {
    handleIncomingAck(frame);
    return;
  }

  Serial.println(F("未知帧类型，已忽略。"));
}

void TelegraphProtocol::handleIncomingData(const char *frame) {
  uint8_t seq = 0;
  char payload[MAX_PAYLOAD_CHARS + 1] = {0};

  if (!parseDataFrame(frame, seq, payload, sizeof(payload))) {
    Serial.println(F("数据帧校验失败或格式错误。"));
    return;
  }

  if (_hasLastRxSeq && seq == _lastRxSeq) {
    Serial.print(F("检测到重复数据帧，重发 ACK，序号="));
    Serial.println(seq, HEX);
    sendAckImmediately(seq);
    return;
  }

  _hasLastRxSeq = true;
  _lastRxSeq = seq;

  Serial.print(F("收到消息: "));
  Serial.println(payload);

  sendAckImmediately(seq);
}

void TelegraphProtocol::handleIncomingAck(const char *frame) {
  uint8_t seq = 0;
  if (!parseAckFrame(frame, seq)) {
    Serial.println(F("ACK 帧校验失败或格式错误。"));
    return;
  }

  if (_state != NodeState::WAIT_ACK) {
    Serial.println(F("当前未等待 ACK，已忽略该 ACK。"));
    return;
  }

  if (seq != _currentSeq) {
    Serial.println(F("ACK 序号不匹配，已忽略。"));
    return;
  }

  Serial.print(F("收到 ACK，序号="));
  Serial.println(seq, HEX);
  finishCurrentMessage();
}

void TelegraphProtocol::sendAckImmediately(uint8_t seq) {
  char ackFrame[MAX_FRAME_CHARS] = {0};
  if (!buildAckFrame(seq, ackFrame, sizeof(ackFrame))) {
    Serial.println(F("ACK 帧构建失败。"));
    return;
  }

  delay(ACK_TURNAROUND_MS);
  Serial.print(F("发送 ACK: "));
  Serial.println(ackFrame);
  _bus.sendTextFrame(ackFrame);
}

void TelegraphProtocol::startArbitrationIfNeeded() {
  if (queueEmpty()) {
    return;
  }

  _state = NodeState::WAIT_IDLE;
  _stateStartMs = millis();
  Serial.println(F("队列中有待发消息，开始等待总线空闲。"));
}

void TelegraphProtocol::handleWaitIdleState(unsigned long now) {
  (void)now;
  if (!_bus.isIdleFor(BUS_IDLE_MS)) {
    return;
  }

  _backoffDelayMs = BACKOFF_BASE_MS + static_cast<unsigned long>(random(0, BACKOFF_RANGE_MS + 1));
  _state = NodeState::BACKOFF;
  _stateStartMs = millis();

  Serial.print(F("检测到总线空闲，进入随机退避，时长="));
  Serial.print(_backoffDelayMs);
  Serial.println(F(" ms"));
}

void TelegraphProtocol::handleBackoffState(unsigned long now) {
  if (!_bus.isIdleFor(BUS_IDLE_MS)) {
    _state = NodeState::WAIT_IDLE;
    Serial.println(F("退避期间检测到总线忙，重新等待空闲。"));
    return;
  }

  if (now - _stateStartMs < _backoffDelayMs) {
    return;
  }

  transmitCurrentDataFrame();
}

void TelegraphProtocol::handleWaitAckState(unsigned long now) {
  if (now - _stateStartMs < ACK_TIMEOUT_MS) {
    return;
  }

  Serial.println(F("等待 ACK 超时。"));
  retryCurrentMessage();
}

void TelegraphProtocol::transmitCurrentDataFrame() {
  const char *payload = queueFront();
  if (payload == nullptr) {
    _state = NodeState::IDLE;
    return;
  }

  if (!buildDataFrame(_currentSeq, payload, _currentFrame, sizeof(_currentFrame))) {
    Serial.println(F("数据帧构建失败，当前消息已丢弃。"));
    queuePopFront();
    _state = NodeState::IDLE;
    return;
  }

  Serial.print(F("开始发送，序号="));
  Serial.println(_currentSeq, HEX);
  Serial.print(F("发送内容: "));
  Serial.println(payload);
  Serial.print(F("发送帧: "));
  Serial.println(_currentFrame);

  _bus.sendTextFrame(_currentFrame);
  _state = NodeState::WAIT_ACK;
  _stateStartMs = millis();
}

void TelegraphProtocol::finishCurrentMessage() {
  queuePopFront();
  _retryCount = 0;
  _currentFrame[0] = '\0';
  _currentSeq = static_cast<uint8_t>(_currentSeq + 1);
  _state = NodeState::IDLE;
  Serial.println(F("当前消息发送完成。"));
}

void TelegraphProtocol::retryCurrentMessage() {
  if (_retryCount >= MAX_RETRY_COUNT) {
    Serial.println(F("超过最大重试次数，当前消息发送失败。"));
    queuePopFront();
    _retryCount = 0;
    _currentFrame[0] = '\0';
    _currentSeq = static_cast<uint8_t>(_currentSeq + 1);
    _state = NodeState::IDLE;
    return;
  }

  ++_retryCount;
  _state = NodeState::WAIT_IDLE;
  _stateStartMs = millis();

  Serial.print(F("准备重试，第 "));
  Serial.print(_retryCount);
  Serial.println(F(" 次。"));
}

bool TelegraphProtocol::queueEmpty() const {
  return _queueCount == 0;
}

bool TelegraphProtocol::queueFull() const {
  return _queueCount >= MESSAGE_QUEUE_SIZE;
}

const char *TelegraphProtocol::queueFront() const {
  if (queueEmpty()) {
    return nullptr;
  }

  return _queue[_queueHead];
}

void TelegraphProtocol::queuePopFront() {
  if (queueEmpty()) {
    return;
  }

  _queue[_queueHead][0] = '\0';
  _queueHead = (_queueHead + 1) % MESSAGE_QUEUE_SIZE;
  --_queueCount;
}
