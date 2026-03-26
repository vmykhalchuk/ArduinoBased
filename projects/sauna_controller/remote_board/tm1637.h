#ifndef TM1637_H
#define TM1637_H

#include <Arduino.h>

namespace TM1637 {

  void init(int pinClk, int pinDio);

  void updateDisplay(uint16_t num, bool showColon, bool displayLeadingZeroes = false, uint8_t brigtness = 3/*0..7*/);
  void updateDisplayWithError(uint8_t errorCode/*0..9 or 0x10 - no code*/ = 0x10, uint8_t brigtness = 3/*0..7*/);

  static void start();
  static void stop();
  static bool writeByte(uint8_t b);
}
#endif
