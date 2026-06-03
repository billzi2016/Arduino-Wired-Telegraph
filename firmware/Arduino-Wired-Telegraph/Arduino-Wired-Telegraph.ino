#include "bus.h"
#include "protocol.h"
#include "telegraph_config.h"

SingleWireBus gBus;
TelegraphProtocol gProtocol(gBus);

namespace {

char gSerialLine[SERIAL_LINE_BUFFER] = {0};
size_t gSerialIndex = 0;

// 逐字节读取串口，一行文本作为一条消息
void readSerialInput() {
  while (Serial.available() > 0) {
    const char ch = static_cast<char>(Serial.read());

    if (ch == '\r') {
      continue;
    }

    if (ch == '\n') {
      gSerialLine[gSerialIndex] = '\0';
      if (gSerialIndex > 0) {
        gProtocol.enqueueUserMessage(gSerialLine);
      }
      gSerialIndex = 0;
      gSerialLine[0] = '\0';
      continue;
    }

    if (gSerialIndex + 1 < sizeof(gSerialLine)) {
      gSerialLine[gSerialIndex++] = ch;
      gSerialLine[gSerialIndex] = '\0';
    }
  }
}

}  // namespace

void setup() {
  Serial.begin(115200);
  gBus.begin(LINE_PIN, BUZZER_PIN, RX_LED_PIN);
  gProtocol.begin();
}

void loop() {
  gBus.tick();
  readSerialInput();
  gProtocol.tick();
}
