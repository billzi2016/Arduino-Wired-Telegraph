#include "morse.h"

#include <ctype.h>
#include <string.h>

namespace {

// 首版支持字母、数字、空格和常见标点
const MorseEntry MORSE_TABLE[] = {
    {'A', ".-"},     {'B', "-..."},   {'C', "-.-."},   {'D', "-.."},
    {'E', "."},      {'F', "..-."},   {'G', "--."},    {'H', "...."},
    {'I', ".."},     {'J', ".---"},   {'K', "-.-"},    {'L', ".-.."},
    {'M', "--"},     {'N', "-."},     {'O', "---"},    {'P', ".--."},
    {'Q', "--.-"},   {'R', ".-."},    {'S', "..."},    {'T', "-"},
    {'U', "..-"},    {'V', "...-"},   {'W', ".--"},    {'X', "-..-"},
    {'Y', "-.--"},   {'Z', "--.."},   {'0', "-----"},  {'1', ".----"},
    {'2', "..---"},  {'3', "...--"},  {'4', "....-"},  {'5', "....."},
    {'6', "-...."},  {'7', "--..."},  {'8', "---.."},  {'9', "----."},
    {'.', ".-.-.-"}, {',', "--..--"}, {'?', "..--.."}, {'\'', ".----."},
    {'!', "-.-.--"}, {'/', "-..-."},  {'(', "-.--."},  {')', "-.--.-"},
    {'&', ".-..."},  {':', "---..."}, {';', "-.-.-."}, {'=', "-...-"},
    {'+', ".-.-."},  {'-', "-....-"}, {'_', "..--.-"}, {'"', ".-..-."},
    {'$', "...-..-"},{'@', ".--.-."},
};

constexpr size_t MORSE_TABLE_SIZE = sizeof(MORSE_TABLE) / sizeof(MORSE_TABLE[0]);

}  // namespace

bool morseEncodeChar(char ch, const char *&code) {
  if (ch == ' ') {
    code = "";
    return true;
  }

  for (size_t i = 0; i < MORSE_TABLE_SIZE; ++i) {
    if (MORSE_TABLE[i].ch == ch) {
      code = MORSE_TABLE[i].code;
      return true;
    }
  }

  code = nullptr;
  return false;
}

char morseDecodeCode(const char *code) {
  for (size_t i = 0; i < MORSE_TABLE_SIZE; ++i) {
    if (strcmp(MORSE_TABLE[i].code, code) == 0) {
      return MORSE_TABLE[i].ch;
    }
  }

  return '?';
}

bool morseIsSupportedChar(char ch) {
  if (ch == ' ') {
    return true;
  }

  const char *unused = nullptr;
  return morseEncodeChar(ch, unused);
}

char morseNormalizeChar(char ch) {
  if (ch == '\t') {
    return ' ';
  }

  if (ch >= 'a' && ch <= 'z') {
    ch = static_cast<char>(toupper(static_cast<unsigned char>(ch)));
  }

  if (morseIsSupportedChar(ch)) {
    return ch;
  }

  return '?';
}
