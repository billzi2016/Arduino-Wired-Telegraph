# Morse Code Reference

## 1. Scope

This document lists the commonly used symbols of International Morse Code for:

- Letters `A-Z`
- Digits `0-9`
- Common punctuation
- Timing rules
- Common procedural signals

If this project needs a strict subset, the recommended first implementation set is:

- `A-Z`
- `0-9`
- Space

---

## 2. Timing Rules

Morse code is defined by time units.

- Dot `.` = `1` unit
- Dash `-` = `3` units
- Gap between parts of the same letter = `1` unit
- Gap between letters = `3` units
- Gap between words = `7` units

Example:

- `A` = `.-`
- Signal pattern:
  - dot `1`
  - intra-character gap `1`
  - dash `3`

For this Arduino project, one unit can be defined as:

- `T = 100 ms`

Then:

- Dot = `100 ms`
- Dash = `300 ms`
- Letter gap = `300 ms`
- Word gap = `700 ms`

---

## 3. Letters

| Character | Morse |
| --- | --- |
| A | `.-` |
| B | `-...` |
| C | `-.-.` |
| D | `-..` |
| E | `.` |
| F | `..-.` |
| G | `--.` |
| H | `....` |
| I | `..` |
| J | `.---` |
| K | `-.-` |
| L | `.-..` |
| M | `--` |
| N | `-.` |
| O | `---` |
| P | `.--.` |
| Q | `--.-` |
| R | `.-.` |
| S | `...` |
| T | `-` |
| U | `..-` |
| V | `...-` |
| W | `.--` |
| X | `-..-` |
| Y | `-.--` |
| Z | `--..` |

---

## 4. Digits

| Character | Morse |
| --- | --- |
| 0 | `-----` |
| 1 | `.----` |
| 2 | `..---` |
| 3 | `...--` |
| 4 | `....-` |
| 5 | `.....` |
| 6 | `-....` |
| 7 | `--...` |
| 8 | `---..` |
| 9 | `----.` |

---

## 5. Common Punctuation

| Character | Morse |
| --- | --- |
| `.` | `.-.-.-` |
| `,` | `--..--` |
| `?` | `..--..` |
| `'` | `.----.` |
| `!` | `-.-.--` |
| `/` | `-..-.` |
| `(` | `-.--.` |
| `)` | `-.--.-` |
| `&` | `.-...` |
| `:` | `---...` |
| `;` | `-.-.-.` |
| `=` | `-...-` |
| `+` | `.-.-.` |
| `-` | `-....-` |
| `_` | `..--.-` |
| `"` | `.-..-.` |
| `$` | `...-..-` |
| `@` | `.--.-.` |

---

## 6. Space Rule

Space is not a Morse symbol by itself.  
It is represented by a word gap:

- Space between words = `7` time units

For software processing, a space can be handled as:

- input character: `' '`
- transmission meaning: word separator

---

## 7. Common Procedural Signals

These are not normal text characters. They are operating signals often used in Morse communication.

| Signal | Meaning | Morse |
| --- | --- | --- |
| `AR` | End of message | `.-.-.` |
| `AS` | Wait | `.-...` |
| `BK` | Break / invite other station | `-...-.-` |
| `BT` | Separator / new section | `-...-` |
| `CL` | Closing down | `-.-..-..` |
| `CT` | Start copying / attention | `-.-.-` |
| `DO` | Change to wabun code | `-..---` |
| `KN` | Go only, named station | `-.--.` |
| `SK` | End of contact | `...-.-` |
| `SOS` | Distress | `...---...` |

Note:

- Some procedural signals can overlap with punctuation patterns depending on notation.
- For this project, procedural signals are optional unless you want protocol-level control symbols.

---

## 8. Extended Latin Letters

These are used in some international variants and may be useful if you want broader coverage later.

| Character | Morse |
| --- | --- |
| À | `.--.-` |
| Ä | `.-.-` |
| Å | `.--.-` |
| Æ | `.-.-` |
| Ą | `.-.-` |
| Ç | `-.-..` |
| Ch | `----` |
| Ð | `..--.` |
| É | `..-..` |
| È | `.-..-` |
| Ĝ | `--.-.` |
| Ĥ | `----` |
| Ĵ | `.---.` |
| Ł | `.-..-` |
| Ñ | `--.--` |
| Ö | `---.` |
| Ø | `---.` |
| Ś | `...-...` |
| Ŝ | `...-.` |
| Þ | `.--..` |
| Ü | `..--` |
| Ŭ | `..--` |
| Ź | `--..-.` |
| Ż | `--..-` |

Note:

- Extended letters are not required for the first version of this project.
- The first version should stay with `A-Z`, `0-9`, and space for simpler encoding and decoding.

---

## 9. Recommended Subset For This Project

To keep the Arduino implementation stable and easy to debug, the recommended supported set is:

- Letters `A-Z`
- Digits `0-9`
- Space

Optional later additions:

- `.`
- `,`
- `?`
- `/`
- `-`

Recommended input normalization:

- Convert lowercase to uppercase before encoding
- Ignore unsupported characters or replace them with `?`

---

## 10. Example Encodings

### HELLO

| Character | Morse |
| --- | --- |
| H | `....` |
| E | `.` |
| L | `.-..` |
| L | `.-..` |
| O | `---` |

Combined by letters:

`.... . .-.. .-.. ---`

### SOS

| Character | Morse |
| --- | --- |
| S | `...` |
| O | `---` |
| S | `...` |

Combined:

`... --- ...`

### ARDUINO

| Character | Morse |
| --- | --- |
| A | `.-` |
| R | `.-.` |
| D | `-..` |
| U | `..-` |
| I | `..` |
| N | `-.` |
| O | `---` |

Combined:

`.- .-. -.. ..- .. -. ---`

---

## 11. Suggested Data Representation In Code

For software implementation, each symbol can be stored as a string:

| Character | Encoded String |
| --- | --- |
| A | `".-"` |
| B | `"-..."` |
| C | `"-.-."` |

For timing-based transmission:

- dot = drive line active for `1T`
- dash = drive line active for `3T`
- symbol gap = idle for `1T`
- letter gap = idle for `3T`
- word gap = idle for `7T`

For lookup tables:

- encoder map: `char -> morse string`
- decoder map: `morse string -> char`

---

## 12. Implementation Notes For This Repository

Recommended first-pass behavior:

1. Read a line from Serial.
2. Convert to uppercase.
3. For each character:
4. Look up the Morse string.
5. Transmit dot/dash pulses on the single-wire bus.
6. Sound the buzzer only while transmitting active pulses.
7. On receive, measure pulse lengths and rebuild the Morse string.
8. Decode complete characters and print them to Serial.

Recommended unsupported-character policy:

- Ignore unsupported characters and print a warning to Serial

or:

- Replace unsupported characters with `?`

---
