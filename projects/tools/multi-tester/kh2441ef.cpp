#include "kh2441ef.h"
#include <util/atomic.h>

namespace KH2441EF {

  inline void _tickV1(uint8_t tickWaitMs);
  inline void _tickV2(uint8_t tickWaitMs);

  inline void _illuminateSingleSegment(uint8_t s);
  inline void _illuminateGroupOfSegments(uint8_t i);
  inline uint8_t __lowSidePinOfSegment(uint8_t s);
  inline uint8_t __highSidePinOfSegment(uint8_t s);
  inline void __clearDAll();
  inline void __drainAll();
  inline void __setDPinHigh(uint8_t p);
  inline void __setDPinLow(uint8_t p);

  uint8_t displayBuf[] = {0,0,0};
  uint8_t state = 0;
  uint16_t startMs = 0;
  uint16_t waitForMs = 0;

  void tick() {
    _tickV2(1500);
  }
  
  inline void _tickV1(uint8_t tickWaitMs = 4) {
    if (state == 0) {
      startMs = ClockLR::now;
      state = 1;
      waitForMs = tickWaitMs;
    }
    
    if (state < 10) { // 1..6
      _illuminateGroupOfSegments(state);
      waitForMs += tickWaitMs;
      state += 10;

    } else if (state > 10) { // 11..16
      if (ClockLR::isElapsed(startMs, waitForMs)) {
        state -= 10;
        state++;
        if (state == 7) state = 0;
      }
    }
  }

  inline void _tickV2(uint8_t tickWaitMs = 2) {
    if (state == 0) {
      startMs = ClockLR::now;
      state = 1;
      waitForMs = tickWaitMs;
    }
    
    if (state < 100) { // 1..24
      _illuminateSingleSegment(state);
      waitForMs += tickWaitMs;
      state += 100;

    } else if (state > 100) { // 101..124
      if (ClockLR::isElapsed(startMs, waitForMs)) {
        state -= 100;
        state++;
        if (state == 8/*25*/) state = 0;
      }
    }
  }

  inline void _illuminateSingleSegment(uint8_t s) {
    __drainAll();
    uint8_t m = 1 << (s%8);
    if (true/*displayBuf[s/8] & m*/) {
      __setDPinHigh(__highSidePinOfSegment(s));
      __setDPinLow(__lowSidePinOfSegment(s));
    }
  }
  
  /**
    group of segments
      1 - {  2,  3,  4,  5,  6 }
      2 - {  7,  8,  9, 14, 15 }
      3 - {  0,  1, 10, 16, 17 }
      4 - { 11, 18, 21 }
      5 - { 12, 19, 22 }
      6 - { 13, 20, 23 }
   */
  inline void _illuminateGroupOfSegments(uint8_t group) {
    __clearDAll();
    if (group == 1) {
      uint8_t segments[] = {2,3,4,5,6};
      for (int j = 0; j < 5; j++) {
        uint8_t s = segments[j];
        uint8_t m = 1 << (s%8);
        if (displayBuf[s/8] & m) __setDPinHigh(__highSidePinOfSegment(s));
      }
    } else if (group == 2) {
      uint8_t segments[] = {7,8,9,14,15};
      for (int j = 0; j < 5; j++) {
        uint8_t s = segments[j];
        uint8_t m = 1 << (s%8);
        if (displayBuf[s/8] & m) __setDPinHigh(__highSidePinOfSegment(s));
      }
    } else if (group == 3) {
      uint8_t segments[] = {0,1,10,16,17};
      for (int j = 0; j < 5; j++) {
        uint8_t s = segments[j];
        uint8_t m = 1 << (s%8);
        if (displayBuf[s/8] & m) __setDPinHigh(__highSidePinOfSegment(s));
      }
    } else if (group == 4) {
      uint8_t segments[] = {11,18,21};
      for (int j = 0; j < 3; j++) {
        uint8_t s = segments[j];
        uint8_t m = 1 << (s%8);
        if (displayBuf[s/8] & m) __setDPinHigh(__highSidePinOfSegment(s));
      }
    } else if (group == 5) {
      uint8_t segments[] = {12,19,22};
      for (int j = 0; j < 3; j++) {
        uint8_t s = segments[j];
        uint8_t m = 1 << (s%8);
        if (displayBuf[s/8] & m) __setDPinHigh(__highSidePinOfSegment(s));
      }
    } else if (group == 6) {
      uint8_t segments[] = {13,20,23};
      for (int j = 0; j < 3; j++) {
        uint8_t s = segments[j];
        uint8_t m = 1 << (s%8);
        if (displayBuf[s/8] & m) __setDPinHigh(__highSidePinOfSegment(s));
      }
    }
    __setDPinLow(group);
  }
  
  const uint8_t __segmentToHighSidePinMap[] =
  //  0&1   2&3   4&5   6&7   8&9   1&1   1&1   1&1   1&1   1&1   2&2   2&2
  //                                0&1   2&3   4&5   6&7   8&9   0&1   2&3
    {0x45, 0x23, 0x45, 0x63, 0x41, 0x11, 0x11, 0x56, 0x62, 0x22, 0x23, 0x33};
  
  const uint8_t __segmentToLowSidePinMap[] =
  //  0&1   2&3   4&5   6&7   8&9   1&1   1&1   1&1   1&1   1&1   2&2   2&2
  //                                0&1   2&3   4&5   6&7   8&9   0&1   2&3
    {0x33, 0x11, 0x11, 0x12, 0x22, 0x34, 0x56, 0x22, 0x33, 0x45, 0x64, 0x56};
    
  // returns 1..6 (pin to pull Low in order to illuminate segment `s`)
  inline uint8_t __lowSidePinOfSegment(uint8_t s) {
    uint8_t pair = __segmentToLowSidePinMap[s>>1];
    if (s % 2 == 0) {
      return pair >> 4;
    } else {
      return pair & 0x0f;
    }
  }
  
  // returns 1..6 (pin to pull High in order to illuminate segment `s`)
  inline uint8_t __highSidePinOfSegment(uint8_t s) {
    uint8_t pair = __segmentToHighSidePinMap[s>>1];
    if (s % 2 == 0) {
      return pair >> 4;
    } else {
      return pair & 0x0f;
    }
  }
  
  inline void __clearDAll() {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      PORTD = PORTD & B00011111;
      PORTB = PORTB & B11111000;
      DDRD = DDRD   & B00011111;
      DDRB = DDRB   & B11111000;
    }
  }

  inline void __drainAll() {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      PORTD = PORTD & B00011111;
      PORTB = PORTB & B11111000;
      DDRD = DDRD   | B11100000;
      DDRB = DDRB   | B00000111;
    }
    delayMicroseconds(100);
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      DDRD = DDRD   & B00011111;
      DDRB = DDRB   & B11111000;
    }
  }
  
  inline void __setDPinHigh(uint8_t p) {
    if (p == 1) {
      DDRD  = DDRD  | B00100000;
      PORTD = PORTD | B00100000;
      
    } else if (p == 2) {
      DDRD  = DDRD  | B01000000;
      PORTD = PORTD | B01000000;
      
    } else if (p == 3) {
      DDRD  = DDRD  | B10000000;
      PORTD = PORTD | B10000000;
      
    } else if (p == 4) {
      DDRB  = DDRB  | B00000001;
      PORTB = PORTB | B00000001;
      
    } else if (p == 5) {
      DDRB  = DDRB  | B00000010;
      PORTB = PORTB | B00000010;
      
    } else if (p == 6) {
      DDRB  = DDRB  | B00000100;
      PORTB = PORTB | B00000100;
    }
  }
  
  inline void __setDPinLow(uint8_t p) {
    if (p == 1) {
      DDRD  = DDRD  | B00100000;
      PORTD = PORTD & B11011111;
      
    } else if (p == 2) {
      DDRD  = DDRD  | B01000000;
      PORTD = PORTD & B10111111;
      
    } else if (p == 3) {
      DDRD  = DDRD  | B10000000;
      PORTD = PORTD & B01111111;
      
    } else if (p == 4) {
      DDRB  = DDRB  | B00000001;
      PORTB = PORTB & B11111110;
      
    } else if (p == 5) {
      DDRB  = DDRB  | B00000010;
      PORTB = PORTB & B11111101;
      
    } else if (p == 6) {
      DDRB  = DDRB  | B00000100;
      PORTB = PORTB & B11111011;
    }
  }

  /*
   Segments: (see KH2441EF-8 pinout.jpg)
    0 - 1b
    1 - 1c
    2 - 2a
    3 - 2b
    4 - 2c
    5 - 2d 
    6 - 2e
    7 - 2f
    8 - 2g
    9 - 3a
   10 - 3b
   11 - 3c
   12 - 3d
   13 - 3e
   14 - 3f
   15 - 3g
   16 - 3dt
   17 - 4a
   18 - 4b
   19 - 4c
   20 - 4d
   21 - 4e
   22 - 4f
   23 - 4g
   
   displayBuf0 - 0-7 segments
   displayBuf1 - 8-15 segments
   displayBuf2 - 16-23 segments
  
   */
    
  void setDisplayBufToInt(uint16_t i) {
    setDisplayBufToInt(i, false);
  }

  void setDisplayBufToInt(uint16_t i, bool displayLeadingZeroes) {
    if (i >= 2000) {
      setDisplayBufToErrorMsg();
      return;
    }
    
    uint8_t d1000 = i / 1000 % 10;
    uint8_t d100 =  i / 100 % 10;
    uint8_t d10 =  i / 10 % 10;
    uint8_t d1 =  i / 1 % 10;
    
    //d100  = d1000 == 0 && d100 == 0 ? 0x10 : d100;
    //d10   = d1000 == 0 && d100 == 0 && d10 == 0 ? 0x10 : d10;

    if (!displayLeadingZeroes) {
      // set leading zeroes to blank
      if (d1000 == 0) {
        d1000 = S_BLANK;
        if (d100 == 0) {
          d100 = S_BLANK;
          if (d10 == 0) {
            d10 = S_BLANK;
          }
        }
      }
    } else {
      d1000 = d1000 == 0 ? S_BLANK : 1;
    }
    
    setDisplayBuf(d1000,d100,d10,d1,false);
  }
  
  void setDisplayBufToIntAsHex(uint16_t i, bool displayLeadingZeroes) {
    if (i >= 0x1FFF) {
      setDisplayBufToErrorMsg();
      return;
    }
    
    uint8_t h1000 = i / 0x1000 % 0x10;
    uint8_t h100 = i / 0x100 % 0x10;
    uint8_t h10 = i / 0x10 % 0x10;
    uint8_t h1 = i / 0x1 % 0x10;

    if (!displayLeadingZeroes) {
      // set leading zeroes to blank
      if (h1000 == 0) {
        h1000 = S_BLANK;
        if (h100 == 0) {
          h100 = S_BLANK;
          if (h10 == 0) {
            h10 = S_BLANK;
          }
        }
      }
    } else {
      h1000 = h1000 == 0 ? S_BLANK : 1;
    }
    
    setDisplayBuf(h1000,h100,h10,h1,false);
  }
  
  void setDisplayBufToFloatWithOneDecimal(float f) {
    f = f < 0 ? -f : f;
    uint8_t d100 = (uint8_t)(f / 100) % 10;
    uint8_t d10 = (uint8_t)(f / 10) % 10;
    uint8_t d1 = (uint8_t)f % 10;
    uint8_t d0 = (uint8_t)(f * 10) % 10;
    if (d100 < 2) {
      setDisplayBuf(d100,d10,d1,d0,true);
    } else {
      setDisplayBuf(S_BLANK,0,S_u,S_r,false); // Ovr - Overflow
    }
  }
  
  void setDisplayBuf(uint8_t dig1, uint8_t dig2, uint8_t dig3, uint8_t dig4, bool point) {
    clearDisplayBuf();
    
    _updateDisplayBufDigitN(4, dig4);
    _updateDisplayBufDigitN(3, dig3);
    _updateDisplayBufDigitN(2, dig2);
    _updateDisplayBufDigitN(1, dig1);
    
    if (point) updateDisplayBufPoint();
  }
  
  void updateDisplayBufDigit(uint8_t digitNo, uint8_t digValue) {
    if (digitNo == 0 || digitNo > 4) return;
    _updateDisplayBufDigitN(digitNo, digValue);
  }
  
  void updateDisplayBufPoint() {
    displayBuf[2] = displayBuf[2] | B0000001;
  }
  
  // Err will be displayed
  void setDisplayBufToErrorMsg() {
    clearDisplayBuf();
    _updateDisplayBufDigitN(2, 0x11); // E
    _updateDisplayBufDigitN(3, 0x12); // r
    _updateDisplayBufDigitN(4, 0x12); // r
  }
  
  void clearDisplayBuf() {
    displayBuf[0] = 0; displayBuf[1] = 0; displayBuf[2] = 0;
  }
  
  // digitNo: [1;4]
  // v:[0;9]; if digitNo==1 v: [0;1]
  void _updateDisplayBufDigitN(uint8_t digitNo, uint8_t v) {
    uint8_t seg = 0;//B0gfedcba
        
    if (v == 0) {
      seg = B00111111;
    } else if (v == 1) {
      seg = B00000110;
    } else if (v == 2) {
      seg = B01011011;
    } else if (v == 3) {
      seg = B01001111;
    } else if (v == 4) {
      seg = B01100110;
    } else if (v == 5) {
      seg = B01101101;
    } else if (v == 6) {
      seg = B01111101;
    } else if (v == 7) {
      seg = B00000111;
    } else if (v == 8) {
      seg = B01111111;
    } else if (v == 9) {
      seg = B01101111;

    //       -a-  
    //     |     |
    //     f     b
    //     |     |
    //       -g-  
    //     |     |
    //     e     c
    //     |     |
    //       -d-

    } else if (v == S_BLANK) {  // (0-------) <blank>
      seg = 0;
    } else if (v == S_E) {      // (0gfed--a) E
      seg = B01111001;
    } else if (v == S_r) {      // (0g-e----) r
      seg = B01010000;
    } else if (v == S_P) {      // (0gfe--ba) P
      seg = B01110011;
    } else if (v == S_SEL1) {   // (0-----b-) Select 1
      seg = B00000010;
    } else if (v == S_SEL2) {   // (0----c--) Select 2
      seg = B00000100;
    } else if (v == S_c) {      // (0g-ed---) c
      seg = B01011000;
    } else if (v == S_C) {      // (0-fed--a) C
      seg = B00111001;
    } else if (v == S_t) {      // (0gfed---) t
      seg = B01111000;
    } else if (v == S_H) {      // (0gfe-cb-) H
      seg = B01110110;
    } else if (v == S_h) {      // (0gfe-c--) h
      seg = B01110100;
    } else if (v == S_UND) {    // (0---d---) _
      seg = B00001000;
    } else if (v == S_MINUS) {  // (0g------) -
      seg = B01000000;
    } else if (v == S_A) {      // (0gfe-cba) A
      seg = B01101101;
    } else if (v == S_b) {      // (0gfedc--) b
      seg = B01111100;
    } else if (v == S_S) {      // (0gf-dc-a) S
      seg = B01101101;
    } else if (v == S_d) {      // (0g-edcb-) d
      seg = B01111111;
    } else if (v == S_u) {      // (0--edc--) u
      seg = B00011100;
    } else if (v == S_F) {      // (0gfe---a) F
      seg = B01110001;
    } else if (v == S_QM) {     // (0g-e--ba) ?
      seg = B01010011;
    }

    //       -a-  
    //     |     |
    //     f     b
    //     |     |
    //       -g-  
    //     |     |
    //     e     c
    //     |     |
    //       -d-

    // Update Display Buffers
    if (digitNo == 1) {
      displayBuf[0] = displayBuf[0] | ((seg >> 1) & B11);
    } else if (digitNo == 2) {
      displayBuf[0] = displayBuf[0] | (seg << 2);
      displayBuf[1] = displayBuf[1] | ((seg & B1000000) >> 6);
    } else if (digitNo == 3) {
      displayBuf[1] = displayBuf[1] | (seg << 1);
    } else if (digitNo == 4) {
      displayBuf[2] = displayBuf[2] | (seg << 1);
    }
  }

  void muteDisplayInstantly() {
    clearDisplayBuf();
    __clearDAll();
  }
}
