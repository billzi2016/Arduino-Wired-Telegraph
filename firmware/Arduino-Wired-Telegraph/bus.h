#ifndef BUS_H
#define BUS_H

#include <Arduino.h>

#include "telegraph_config.h"

// 单线总线层：负责开漏发送、边沿接收和莫尔斯符号切分
class SingleWireBus {
 public:
  void begin(uint8_t linePin, uint8_t buzzerPin, uint8_t rxLedPin);
  void tick();

  // 判断总线是否已经空闲足够长时间，供仲裁使用
  bool isIdleFor(unsigned long idleMs) const;

  // 发送一整段莫尔斯文本，调用方需先完成仲裁
  bool sendTextFrame(const char *text);

  // 取出一帧已经接收完成的文本
  bool popReceivedFrame(char *out, size_t outSize);

 private:
  void driveActive();
  void releaseLine();
  void setBuzzer(bool on);
  void setRxLed(bool on);

  void startFrame();
  void finishFrame();
  void appendSymbolChar(char symbol);
  void flushCurrentSymbol(bool appendSpace);
  void appendFrameChar(char ch);
  void handleLowPulse(unsigned long durationMs);
  void handleHighGap(unsigned long durationMs);

  uint8_t _linePin = LINE_PIN;
  uint8_t _buzzerPin = BUZZER_PIN;
  uint8_t _rxLedPin = RX_LED_PIN;

  bool _lastLevelHigh = true;
  unsigned long _lastEdgeMs = 0;
  bool _frameActive = false;

  char _currentSymbol[12] = {0};
  size_t _currentSymbolLen = 0;

  char _frameBuffer[MAX_FRAME_CHARS] = {0};
  size_t _frameLen = 0;

  char _readyFrame[MAX_FRAME_CHARS] = {0};
  bool _hasReadyFrame = false;
};

#endif
