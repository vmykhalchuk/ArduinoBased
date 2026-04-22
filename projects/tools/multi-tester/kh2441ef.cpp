#include "kh2441ef.h"

namespace KH2441EF {

  uint8_t displayBuf[] = {0,0,0};
  uint8_t state = 1;
  uint16_t startMs = 0;
  uint8_t waitForMs = 0;

  void tick() {
    if (state == 0) {
      startMs = ClockLR::now;
      state++;
    }
    
    if (state < 10) { // 1..6
      displayUpdateForBatch(state);
      waitForMs = state * 4;
      state += 10;

    } else if (state > 10) { // 11..16
      if (ClockLR::isElapsed(startMs, waitForMs)) {
        state -= 10;
        state++;
        if (state == 7) state = 0;
      }
    }
  }

  void displayLoop24ms() {
    uint16_t startMs = ClockLR::now;
    for (int i = 1; i <= 6; i++) {
      displayUpdateForBatch(i);
      while (!ClockLR::isElapsed(startMs, i*4)) {};
    }
  }
  
  /**
      1 - { 2, 3, 4, 5, 6}
      2 - { 7, 8, 9,14,15}
      3 - { 0, 1,10,16,17}
      4 - {11,18,21}
      5 - {12,19,22}
      6 - {13,20,23}
   */
  void displayUpdateForBatch(uint8_t i) {
    clearDAll();
    if (i == 1) {
      uint8_t segments[] = {2,3,4,5,6};
      for (int j = 0; j < 5; j++) {
        uint8_t s = segments[j];
        uint8_t m = 1 << (s%8);
        if (displayBuf[s/8] & m) lightDSegmentOnlyHigh(s);
      }
    } else if (i == 2) {
      uint8_t segments[] = {7,8,9,14,15};
      for (int j = 0; j < 5; j++) {
        uint8_t s = segments[j];
        uint8_t m = 1 << (s%8);
        if (displayBuf[s/8] & m) lightDSegmentOnlyHigh(s);
      }
    } else if (i == 3) {
      uint8_t segments[] = {0,1,10,16,17};
      for (int j = 0; j < 5; j++) {
        uint8_t s = segments[j];
        uint8_t m = 1 << (s%8);
        if (displayBuf[s/8] & m) lightDSegmentOnlyHigh(s);
      }
    } else if (i == 4) {
      uint8_t segments[] = {11,18,21};
      for (int j = 0; j < 3; j++) {
        uint8_t s = segments[j];
        uint8_t m = 1 << (s%8);
        if (displayBuf[s/8] & m) lightDSegmentOnlyHigh(s);
      }
    } else if (i == 5) {
      uint8_t segments[] = {12,19,22};
      for (int j = 0; j < 3; j++) {
        uint8_t s = segments[j];
        uint8_t m = 1 << (s%8);
        if (displayBuf[s/8] & m) lightDSegmentOnlyHigh(s);
      }
    } else if (i == 6) {
      uint8_t segments[] = {13,20,23};
      for (int j = 0; j < 3; j++) {
        uint8_t s = segments[j];
        uint8_t m = 1 << (s%8);
        if (displayBuf[s/8] & m) lightDSegmentOnlyHigh(s);
      }
    }
    setDPinLow(i);
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
  void lightDSegmentOnlyHigh(uint8_t s) {
    if (s == 0) {
      setDPinHigh(4); // 3 Low
    } else if (s == 1) {
      setDPinHigh(5); // 3 Low
    } else if (s == 2) {
      setDPinHigh(2); // 1 Low
    } else if (s == 3) {
      setDPinHigh(3); // 1 Low
    } else if (s == 4) {
      setDPinHigh(4); // 1 Low
    } else if (s == 5) {
      setDPinHigh(5); // 1 Low
    } else if (s == 6) {
      setDPinHigh(6); // 1 Low
    } else if (s == 7) {
      setDPinHigh(3); // 2 Low
    } else if (s == 8) {
      setDPinHigh(4); // 2 Low
    } else if (s == 9) {
      setDPinHigh(1); // 2 Low
    } else if (s == 10) {
      setDPinHigh(1); // 3 Low
    } else if (s == 11) {
      setDPinHigh(1); // 4 Low
    } else if (s == 12) {
      setDPinHigh(1); // 5 Low
    } else if (s == 13) {
      setDPinHigh(1); // 6 Low
    } else if (s == 14) {
      setDPinHigh(5); // 2 Low
    } else if (s == 15) {
      setDPinHigh(6); // 2 Low
    } else if (s == 16) {
      setDPinHigh(6); // 3 Low
    } else if (s == 17) {
      setDPinHigh(2); // 3 Low
    } else if (s == 18) {
      setDPinHigh(2); // 4 Low
    } else if (s == 19) {
      setDPinHigh(2); // 5 Low
    } else if (s == 20) {
      setDPinHigh(2); // 6 Low
    } else if (s == 21) {
      setDPinHigh(3); // 4 Low
    } else if (s == 22) {
      setDPinHigh(3); // 5 Low
    } else if (s == 23) {
      setDPinHigh(3); // 6 Low
    }
  }
  
  void clearDAll() {
    DDRD = DDRD & B00011111;
    PORTD = PORTD & B00011111;
    DDRB = DDRB & B11111000;
    PORTB = PORTB & B11111000;
  }
  
  void setDPinHigh(uint8_t p) {
    if (p == 1) {
      DDRD = DDRD | B00100000;
      PORTD = PORTD | B00100000;
    } else if (p == 2) {
      DDRD = DDRD | B01000000;
      PORTD = PORTD | B01000000;
    } else if (p == 3) {
      DDRD = DDRD | B10000000;
      PORTD = PORTD | B10000000;
    } else if (p == 4) {
      DDRB = DDRB | B00000001;
      PORTB = PORTB | B00000001;
    } else if (p == 5) {
      DDRB = DDRB | B00000010;
      PORTB = PORTB | B00000010;
    } else if (p == 6) {
      DDRB = DDRB | B00000100;
      PORTB = PORTB | B00000100;
    }
  }
  
  void setDPinLow(uint8_t p) {
    if (p == 1) {
      DDRD = DDRD | B00100000;
      PORTD = PORTD & B11011111;
    } else if (p == 2) {
      DDRD = DDRD | B01000000;
      PORTD = PORTD & B10111111;
    } else if (p == 3) {
      DDRD = DDRD | B10000000;
      PORTD = PORTD & B01111111;
    } else if (p == 4) {
      DDRB = DDRB | B00000001;
      PORTB = PORTB & B11111110;
    } else if (p == 5) {
      DDRB = DDRB | B00000010;
      PORTB = PORTB & B11111101;
    } else if (p == 6) {
      DDRB = DDRB | B00000100;
      PORTB = PORTB & B11111011;
    }
  }

  void setDisplayBufToInt(uint16_t i) {
    if (i >= 2000) {
      setDisplayBufToErrorMsg();
      return;
    }
    uint8_t d1000 = i / 1000 % 10;
    uint8_t d100 =  i / 100 % 10;
    uint8_t d10 =  i / 10 % 10;
    uint8_t d1 =  i / 1 % 10;
    d1000 = d1000 == 0 ? 0x10 : 1;
    d100  = d1000 == 0 && d100 == 0 ? 0x10 : d100;
    d10   = d1000 == 0 && d100 == 0 && d10 == 0 ? 0x10 : d10;
    setDisplayBuf(d1000,d100,d10,d1,false);
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
      setDisplayBuf(0,9,9,9,false);
    }
  }
  
  void setDisplayBuf(uint8_t dig1, uint8_t dig2, uint8_t dig3, uint8_t dig4, bool point) {
    clearDisplayBuf();
    
    updateDisplayBufDigitN(4, dig4);
    if (dig3 != 0 || point || dig1 != 0 || dig2 != 0) {
      updateDisplayBufDigitN(3, dig3);
    }
    if ((dig2 != 0) || (dig1 != 0)) {
      updateDisplayBufDigitN(2, dig2);
    }
    if (dig1 == 1) {
      displayBuf[0] = displayBuf[0] | B0000011;
    }
    if (point) {
      displayBuf[2] = displayBuf[2] | B0000001;
    }
  }
  
  // Err will be displayed
  void setDisplayBufToErrorMsg() {
    clearDisplayBuf();
    updateDisplayBufDigitN(2, 0x11); // E
    updateDisplayBufDigitN(3, 0x12); // r
    updateDisplayBufDigitN(4, 0x12); // r
  }
  
  void clearDisplayBuf() {
    displayBuf[0] = 0; displayBuf[1] = 0; displayBuf[2] = 0;
  }
  
  // digitNo: [1;4]
  // v:[0;9]; if digitNo==1 v: [0;1]
  void updateDisplayBufDigitN(uint8_t digitNo, uint8_t v) {
    uint8_t seg = 0;//B0gfedcba
        
    //       -a-  
    //     |     |
    //     f     b
    //     |     |
    //       -g-  
    //     |     |
    //     e     c
    //     |     |
    //       -d-

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

    } else if (v == S_BLANK) {  // <none> ()
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
    }

    if (digitNo == 1) {
      displayBuf[0] = displayBuf[0] | (seg << 1) & B11;
    } else if (digitNo == 2) {
      displayBuf[0] = displayBuf[0] | seg << 2;
      displayBuf[1] = displayBuf[1] | (seg & B1000000) >> 6;
    } else if (digitNo == 3) {
      displayBuf[1] = displayBuf[1] | seg << 1;
    } else if (digitNo == 4) {
      displayBuf[2] = displayBuf[2] | seg << 1;
    }
  }

  void muteDisplayInstantly() {
    clearDisplayBuf();
    clearDAll();
  }
}
