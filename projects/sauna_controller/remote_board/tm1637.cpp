#include "tm1637.h"

namespace TM1637 {

  /*
     -a-
   |     |
   f     b  dt
   |     |
     -g-
   |     |  dt
   e     c
   |     |
     -d-   

   Bits: [dt][g][f][e][d][c][b][a]
          dt - only for d1
   */

  // Seven-segment patterns for 0-9
  static const uint8_t SEGMENT_MAP[] = {
    0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f
  };

  static const uint8_t SEGMENT_MAP_E = 0x79;
  static const uint8_t SEGMENT_MAP_R = 0x50;
  
  static bool _isInitialized = false;
  static int _pinClk;
  static int _pinDio;
  
  void init(int pinClk, int pinDio) {
    _pinClk = pinClk;
    _pinDio = pinDio;
    pinMode(_pinClk, OUTPUT);
    pinMode(_pinDio, OUTPUT);
    _isInitialized = true;
  }
  
  // --- Low Level Communication ---
  
  static void start() {
    digitalWrite(_pinClk, HIGH);
    digitalWrite(_pinDio, HIGH);
    delayMicroseconds(5); // can be 2 (try it first), same for all 5us
    digitalWrite(_pinDio, LOW);
  }
  
  static void stop() {
    digitalWrite(_pinClk, LOW);
    digitalWrite(_pinDio, LOW);
    delayMicroseconds(5);
    digitalWrite(_pinClk, HIGH);
    digitalWrite(_pinDio, HIGH);
    delayMicroseconds(5);
  }
  
  static bool writeByte(uint8_t b) {
    for (uint8_t i = 0; i < 8; i++) {
      digitalWrite(_pinClk, LOW);
      digitalWrite(_pinDio, (b & 0x01) ? HIGH : LOW); // Send LSB first
      b >>= 1;
      delayMicroseconds(5);
      digitalWrite(_pinClk, HIGH);
      delayMicroseconds(5);
    }
    
    // Wait for ACK
    digitalWrite(_pinClk, LOW);
    pinMode(_pinDio, INPUT);
    delayMicroseconds(5);
    digitalWrite(_pinClk, HIGH);
    delayMicroseconds(5);
    bool ack = digitalRead(_pinDio) == 0;
    pinMode(_pinDio, OUTPUT);
    return ack;
  }
  
  // --- High Level Functions ---
  
  void _updateDisplay(uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t brigtness) {
    if (!_isInitialized) return;
  
    // 1. Data Command: Setting automatic address adding
    start();
    writeByte(0x40);
    stop();
  
    // 2. Address Command: Set start address to 0xC0 (first digit)
    start();
    writeByte(0xC0);
    writeByte(d0);
    writeByte(d1);
    writeByte(d2);
    writeByte(d3);
    stop();
  
    // 3. Display Control: Turn on display, set brightness (0x88 is on, 0x8f is max)
    start();
    writeByte(0x88 + (brigtness & 7)); // 0x88 is "ON", +7 is max brightness
    stop();
  }

  void updateDisplay(uint16_t num, bool showColon, bool displayLeadingZeroes, uint8_t brigtness) {
    uint8_t digits[4];
    digits[0] = SEGMENT_MAP[(num / 1000) % 10];
    digits[1] = SEGMENT_MAP[(num / 100) % 10];
    digits[2] = SEGMENT_MAP[(num / 10) % 10];
    digits[3] = SEGMENT_MAP[num % 10];

    for (uint8_t i = 0; i < 3; i++) {
      if (!displayLeadingZeroes && digits[i] == SEGMENT_MAP[0]) digits[i] = 0;
      else break;
    }
  
    // apply colon to second digit
    if (showColon) {
      digits[1] |= 0x80;
    }

    _updateDisplay(digits[0], digits[1], digits[2], digits[3], brigtness);
  }

  void updateDisplayWithError(uint8_t errorCode, uint8_t brigtness) {
    _updateDisplay(SEGMENT_MAP_E, SEGMENT_MAP_E, SEGMENT_MAP_R, errorCode == 0x10 ? 0 : errorCode % 10, brigtness);
  }
  
}
