#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <Arduino.h>

#include "bus.h"

// 节点协议状态
enum class NodeState : uint8_t {
  IDLE,
  WAIT_IDLE,
  BACKOFF,
  WAIT_ACK,
};

// 上层协议：负责排队、仲裁、重发、ACK 和串口日志
class TelegraphProtocol {
 public:
  explicit TelegraphProtocol(SingleWireBus &bus);

  void begin();
  void tick();

  // 串口一行文本入队，后续会自动发送
  bool enqueueUserMessage(const char *rawLine);

 private:
  bool sanitizeMessage(const char *rawLine, char *out, size_t outSize);
  bool enqueueSanitized(const char *message);
  bool buildDataFrame(uint8_t seq, const char *payload, char *out, size_t outSize) const;
  bool buildAckFrame(uint8_t seq, char *out, size_t outSize) const;
  bool parseDataFrame(const char *frame, uint8_t &seq, char *payload, size_t payloadSize) const;
  bool parseAckFrame(const char *frame, uint8_t &seq) const;

  uint8_t computeFrameCrc(const char *text, size_t length) const;
  void byteToHex(uint8_t value, char *out) const;
  bool hexToByte(const char *text, uint8_t &value) const;

  void processIncomingFrames();
  void handleIncomingFrame(const char *frame);
  void handleIncomingData(const char *frame);
  void handleIncomingAck(const char *frame);
  void sendAckImmediately(uint8_t seq);

  void startArbitrationIfNeeded();
  void handleWaitIdleState(unsigned long now);
  void handleBackoffState(unsigned long now);
  void handleWaitAckState(unsigned long now);
  void transmitCurrentDataFrame();
  void finishCurrentMessage();
  void retryCurrentMessage();

  bool queueEmpty() const;
  bool queueFull() const;
  const char *queueFront() const;
  void queuePopFront();

  SingleWireBus &_bus;
  NodeState _state = NodeState::IDLE;

  char _queue[MESSAGE_QUEUE_SIZE][MAX_PAYLOAD_CHARS + 1] = {{0}};
  size_t _queueHead = 0;
  size_t _queueTail = 0;
  size_t _queueCount = 0;

  char _currentFrame[MAX_FRAME_CHARS] = {0};
  uint8_t _currentSeq = 0;
  uint8_t _retryCount = 0;
  unsigned long _stateStartMs = 0;
  unsigned long _backoffDelayMs = 0;

  bool _hasLastRxSeq = false;
  uint8_t _lastRxSeq = 0;
};

#endif
