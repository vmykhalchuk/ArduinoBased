#include "tests.h"

namespace Tests {
  static SwitchDef swLed1, swBuzzer;

  void init(SwitchDef swPanelLed1, SwitchDef swPanelBuzzer) {
    swLed1 = swPanelLed1;
    swBuzzer = swPanelBuzzer;
  }

  void blinkTestNo(uint8_t testNo) {
    for (uint8_t i = 0; i < testNo; i++) {
      delay(150);
      switchOn(swLed1);
      switchOn(swBuzzer);
      delay(150);
      switchOff(swLed1);
      switchOff(swBuzzer);
    }
    delay(150);
  }
}
