# Arduino Single-Wire Morse Telegraph

This project is a single-wire, half-duplex wired telegraph built with two Arduino Uno boards.  
Both boards run the **same firmware** and communicate through one data wire plus a shared ground. Frame contents are transmitted with Morse code, with basic bus arbitration, ACK confirmation, and timeout-based retransmission.

## Features

- Symmetric design with two Arduino Uno boards and no master/slave split
- Single-wire half-duplex communication
- Automatically sends a line of text entered through the serial port
- Uses Morse code for transmission
- Drives a buzzer on `D13` while sending
- Lights a green LED on `D12` while receiving
- Supports bus idle detection, random backoff, ACK, and timeout retransmission

## Directory Structure

- `firmware/Arduino-Wired-Telegraph/`
  - Arduino project directory
  - Open this directory directly in the Arduino IDE
- `docs/`
  - `PRD.md`: product requirements document
  - `MORSE_CODE.md`: Morse code reference table
  - `MORSE_TREE.md`: Morse code tree
  - `WIRING.md`: wiring instructions
  - `AI_OBSERVE.md`: notes from AI-assisted development

## Firmware Layout

The main files under `firmware/Arduino-Wired-Telegraph/` are:

- `Arduino-Wired-Telegraph.ino`
  - Main entry point responsible for initialization and the main loop
- `telegraph_config.h`
  - Global pin definitions and timing configuration
- `bus.h` / `bus.cpp`
  - Single-wire bus layer for open-drain style transmission, receive-edge parsing, and symbol splitting
- `morse.h` / `morse.cpp`
  - Morse code table and character encode/decode logic
- `protocol.h` / `protocol.cpp`
  - Upper-layer protocol including message queuing, bus arbitration, ACK, retransmission, and serial logging

## Default Hardware Mapping

For each Arduino Uno:

- `D2`: single-wire communication pin
- `D13`: buzzer control output
- `D12`: green LED output for receive indication

Between the two boards:

- One data wire connects `D2 <-> D2`
- One shared ground wire connects `GND <-> GND`

Notes:

- Although the protocol is logically "single-wire communication," the hardware **must** share a common ground
- The program follows an open-drain or quasi-open-drain approach: it pulls the bus low when sending an active signal, and releases the bus when idle while relying on pull-up behavior

## Usage

1. Open `firmware/Arduino-Wired-Telegraph/Arduino-Wired-Telegraph.ino` in the Arduino IDE.
2. Flash the same firmware to both Arduino Uno boards.
3. Connect the data wire, shared ground, buzzer, and receive LED as specified.
4. Open the serial monitor on both sides and set the baud rate to `115200`.
5. Enter a line of text on either side and press Enter.
6. The local board sends automatically, and the remote board receives and prints the message through serial output.

## Current Protocol Behavior

- Both devices run symmetrically
- The sender checks whether the bus is idle before transmitting
- If both sides want to send at the same time, random backoff is used to avoid collision
- The receiver automatically sends an ACK after valid data is received
- The sender automatically retries after an ACK timeout
- The default maximum length of a single message is `32` characters

## Notes

- Comments in the current codebase are written in Chinese
- The repository already includes protocol design notes and Morse reference documents
- Hardware wiring details are documented in `docs/WIRING.md`
- The AI collaboration process is documented in `docs/AI_OBSERVE.md`
- If you want to extend the project further, the recommended next step is to add wiring diagrams, a state machine diagram, and real debugging records
