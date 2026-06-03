#ifndef MORSE_H
#define MORSE_H

#include <Arduino.h>

// 单个字符与莫尔斯码的映射项
struct MorseEntry {
  char ch;
  const char *code;
};

// 将字符编码成莫尔斯码，成功返回 true
bool morseEncodeChar(char ch, const char *&code);

// 将莫尔斯码解码成字符，失败返回 '?'
char morseDecodeCode(const char *code);

// 判断字符是否在当前支持集合内
bool morseIsSupportedChar(char ch);

// 归一化字符：小写转大写，制表符转空格，不支持字符转 '?'
char morseNormalizeChar(char ch);

#endif
