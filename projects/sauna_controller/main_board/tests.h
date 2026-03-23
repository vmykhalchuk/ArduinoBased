#ifndef TESTS_H
#define TESTS_H

#include <Arduino.h>
#include "switch_pin.h"
#include "rs485_server.h"

namespace Tests {

  void runTests(SwitchDef buzzer, SwitchDef fanMain, SwitchDef fanTRIACs, SwitchDef heaterTRIACs,
                SwitchDef rel1, SwitchDef rel2, SwitchDef rel3, SwitchDef rel4);
                
  void testDataRefresh(SwitchDef buzzer, SwitchDef rel1, SwitchDef rel2, SwitchDef rel3, SwitchDef rel4);
}

#endif
