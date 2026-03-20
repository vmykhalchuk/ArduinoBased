#ifndef TM1637_H
#define TM1637_H

#include <Arduino.h>

namespace TM1637 {

  void init(int pinClk, int pinDio);

  void updateDisplay(int num, bool showColon);

  static void start();
  static void stop();
  static bool writeByte(uint8_t b);
}
#endif
