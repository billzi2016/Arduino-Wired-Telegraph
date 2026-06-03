#include "bus.h"

#include <string.h>

#include "morse.h"

void SingleWireBus::begin(uint8_t linePin, uint8_t buzzerPin, uint8_t rxLedPin) {
  _linePin = linePin;
  _buzzerPin = buzzerPin;
  _rxLedPin = rxLedPin;

  pinMode(_linePin, INPUT_PULLUP);
  pinMode(_buzzerPin, OUTPUT);
  pinMode(_rxLedPin, OUTPUT);

  digitalWrite(_buzzerPin, LOW);
  digitalWrite(_rxLedPin, LOW);

  _lastLevelHigh = (digitalRead(_linePin) == HIGH);
  _lastEdgeMs = millis();
}

void SingleWireBus::tick() {
  const unsigned long now = millis();
  const bool levelHigh = (digitalRead(_linePin) == HIGH);

  if (levelHigh != _lastLevelHigh) {
    const unsigned long durationMs = now - _lastEdgeMs;

    if (_lastLevelHigh) {
      handleHighGap(durationMs);
    } else {
      handleLowPulse(durationMs);
    }

    _lastLevelHigh = levelHigh;
    _lastEdgeMs = now;
  }

  // 若已经持续高电平很久，则认为这一帧真正结束
  if (_frameActive && _lastLevelHigh && (now - _lastEdgeMs >= FRAME_END_GAP_MS)) {
    flushCurrentSymbol(false);
    finishFrame();
  }
}

bool SingleWireBus::isIdleFor(unsigned long idleMs) const {
  if (_frameActive) {
    return false;
  }

  if (digitalRead(_linePin) == LOW) {
    return false;
  }

  return (millis() - _lastEdgeMs) >= idleMs;
}

bool SingleWireBus::sendTextFrame(const char *text) {
  if (text == nullptr || text[0] == '\0') {
    return false;
  }

  setRxLed(false);

  for (size_t i = 0; text[i] != '\0'; ++i) {
    const char current = text[i];

    if (current == ' ') {
      continue;
    }

    const char *code = nullptr;
    if (!morseEncodeChar(current, code) || code == nullptr) {
      continue;
    }

    const size_t codeLen = strlen(code);
    for (size_t j = 0; j < codeLen; ++j) {
      driveActive();
      setBuzzer(true);
      delay(code[j] == '.' ? DOT_MS : DASH_MS);

      releaseLine();
      setBuzzer(false);

      if (j + 1 < codeLen) {
        delay(SYMBOL_GAP_MS);
      }
    }

    // 根据后续字符决定是字母间隔还是单词间隔
    bool hasNextVisibleChar = false;
    bool nextIsSpace = false;
    for (size_t next = i + 1; text[next] != '\0'; ++next) {
      if (text[next] == ' ') {
        nextIsSpace = true;
        continue;
      }

      hasNextVisibleChar = true;
      break;
    }

    if (hasNextVisibleChar) {
      delay(nextIsSpace ? WORD_GAP_MS : LETTER_GAP_MS);
    }
  }

  releaseLine();
  setBuzzer(false);
  return true;
}

bool SingleWireBus::popReceivedFrame(char *out, size_t outSize) {
  if (!_hasReadyFrame || out == nullptr || outSize == 0) {
    return false;
  }

  strncpy(out, _readyFrame, outSize - 1);
  out[outSize - 1] = '\0';
  _hasReadyFrame = false;
  _readyFrame[0] = '\0';
  return true;
}

void SingleWireBus::driveActive() {
  pinMode(_linePin, OUTPUT);
  digitalWrite(_linePin, LOW);
}

void SingleWireBus::releaseLine() {
  pinMode(_linePin, INPUT_PULLUP);
}

void SingleWireBus::setBuzzer(bool on) {
  digitalWrite(_buzzerPin, on ? HIGH : LOW);
}

void SingleWireBus::setRxLed(bool on) {
  digitalWrite(_rxLedPin, on ? HIGH : LOW);
}

void SingleWireBus::startFrame() {
  _frameActive = true;
  _frameLen = 0;
  _frameBuffer[0] = '\0';
  _currentSymbolLen = 0;
  _currentSymbol[0] = '\0';
  setRxLed(true);
}

void SingleWireBus::finishFrame() {
  _frameActive = false;
  setRxLed(false);

  if (_frameLen == 0) {
    return;
  }

  strncpy(_readyFrame, _frameBuffer, sizeof(_readyFrame) - 1);
  _readyFrame[sizeof(_readyFrame) - 1] = '\0';
  _hasReadyFrame = true;

  _frameLen = 0;
  _frameBuffer[0] = '\0';
  _currentSymbolLen = 0;
  _currentSymbol[0] = '\0';
}

void SingleWireBus::appendSymbolChar(char symbol) {
  if (_currentSymbolLen + 1 >= sizeof(_currentSymbol)) {
    return;
  }

  _currentSymbol[_currentSymbolLen++] = symbol;
  _currentSymbol[_currentSymbolLen] = '\0';
}

void SingleWireBus::flushCurrentSymbol(bool appendSpace) {
  if (_currentSymbolLen > 0) {
    const char decoded = morseDecodeCode(_currentSymbol);
    appendFrameChar(decoded);
    _currentSymbolLen = 0;
    _currentSymbol[0] = '\0';
  }

  if (appendSpace && _frameLen > 0 && _frameBuffer[_frameLen - 1] != ' ') {
    appendFrameChar(' ');
  }
}

void SingleWireBus::appendFrameChar(char ch) {
  if (_frameLen + 1 >= sizeof(_frameBuffer)) {
    return;
  }

  _frameBuffer[_frameLen++] = ch;
  _frameBuffer[_frameLen] = '\0';
}

void SingleWireBus::handleLowPulse(unsigned long durationMs) {
  if (!_frameActive) {
    startFrame();
  }

  if (durationMs < (UNIT_MS / 2)) {
    return;
  }

  appendSymbolChar(durationMs < DOT_DASH_THRESHOLD_MS ? '.' : '-');
}

void SingleWireBus::handleHighGap(unsigned long durationMs) {
  if (!_frameActive) {
    return;
  }

  if (durationMs < INTRA_SYMBOL_MAX_MS) {
    return;
  }

  if (durationMs < WORD_GAP_THRESHOLD_MS) {
    flushCurrentSymbol(false);
    return;
  }

  if (durationMs < FRAME_END_GAP_MS) {
    flushCurrentSymbol(true);
    return;
  }

  flushCurrentSymbol(false);
  finishFrame();
}
