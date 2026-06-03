#ifndef TELEGRAPH_CONFIG_H
#define TELEGRAPH_CONFIG_H

#include <Arduino.h>

// 引脚定义：两块板子烧录同一份程序，硬件保持完全对称
constexpr uint8_t LINE_PIN = 2;
constexpr uint8_t BUZZER_PIN = 13;
constexpr uint8_t RX_LED_PIN = 12;

// 莫尔斯时间基准
constexpr unsigned long UNIT_MS = 100;
constexpr unsigned long DOT_MS = UNIT_MS;
constexpr unsigned long DASH_MS = UNIT_MS * 3;
constexpr unsigned long SYMBOL_GAP_MS = UNIT_MS;
constexpr unsigned long LETTER_GAP_MS = UNIT_MS * 3;
constexpr unsigned long WORD_GAP_MS = UNIT_MS * 7;

// 接收判定阈值
constexpr unsigned long DOT_DASH_THRESHOLD_MS = UNIT_MS * 2;
constexpr unsigned long INTRA_SYMBOL_MAX_MS = UNIT_MS * 2;
constexpr unsigned long WORD_GAP_THRESHOLD_MS = UNIT_MS * 6;
constexpr unsigned long FRAME_END_GAP_MS = UNIT_MS * 12;
constexpr unsigned long BUS_IDLE_MS = UNIT_MS * 10;

// 协议参数
constexpr unsigned long ACK_TURNAROUND_MS = UNIT_MS * 2;
constexpr unsigned long ACK_TIMEOUT_MS = UNIT_MS * 80;
constexpr unsigned long BACKOFF_BASE_MS = UNIT_MS * 2;
constexpr unsigned long BACKOFF_RANGE_MS = UNIT_MS * 5;
constexpr uint8_t MAX_RETRY_COUNT = 3;

// 缓冲区大小
constexpr size_t MAX_PAYLOAD_CHARS = 32;
constexpr size_t MAX_FRAME_CHARS = 96;
constexpr size_t SERIAL_LINE_BUFFER = 80;
constexpr size_t MESSAGE_QUEUE_SIZE = 4;

#endif
