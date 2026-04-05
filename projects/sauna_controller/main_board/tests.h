#ifndef TESTS_H
#define TESTS_H

#include <Arduino.h>
#include "switch_pin.h"
#include "rs485_server.h"

namespace Tests {

  //void init(SwitchDef buzzer, SwitchDef fanMain, SwitchDef fanTRIACs, SwitchDef heaterTRIACs,
  //              SwitchDef rel1, SwitchDef rel2, SwitchDef rel3, SwitchDef rel4, int pin_TestBtn);

  void runTests(SwitchDef &buzzer, SwitchDef &fanMain, SwitchDef &fanTRIACs, SwitchDef &heaterTRIACs,
                SwitchDef &rel1, SwitchDef &rel2, SwitchDef &rel3, SwitchDef &rel4, int pin_TestBtn);
                
  void testDataRefresh(SwitchDef &sw_InfoPanel_Buzzer, 
                       SwitchDef &sw_Relay1_ALARM, SwitchDef &sw_Relay2_HEAT_FAN,
                       SwitchDef &sw_Relay3_POWER, SwitchDef &sw_Relay4);
}

#endif
