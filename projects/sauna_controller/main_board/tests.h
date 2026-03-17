#ifndef TESTS_H
#define TESTS_H

#include <Arduino.h>
#include "switch_pin.h"

namespace Tests {

  void init(SwitchDef swPanelLed1, SwitchDef swPanelBuzzer);
  void blinkTestNo(uint8_t testNo);
}

#endif
