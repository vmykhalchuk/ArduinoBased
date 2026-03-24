#include "tests.h"

namespace Tests {

  //            sw_InfoPanel_Buzzer, sw_fan_Main, sw_fan_TRIACs, sw_Heater_TRIACs,
  //            sw_Relay1_ALARM, sw_Relay2_HEAT_FAN, sw_Relay3_POWER, sw_Relay4
  void runTests(SwitchDef buzzer, SwitchDef fanMain, SwitchDef fanTRIACs, SwitchDef heaterTRIACs,
                SwitchDef rel1, SwitchDef rel2, SwitchDef rel3, SwitchDef rel4) {
    for (uint8_t i = 1; i <= 5; i++) {
      blink(buzzer, i);
      // FIXME Make it more solid test - to check all systems working
      switch (i) {
        case 1:
          delay(1000);
          switchOn(fanMain);
          delay(3000);
          switchOff(fanMain);
          break;
        case 2:
          delay(1000);
          switchOn(fanTRIACs);
          delay(3000);
          switchOff(fanTRIACs);
          break;
        case 3:
          delay(1000);
          switchOn(heaterTRIACs);
          delay(3000);
          switchOff(heaterTRIACs);
          break;
        case 4:
          delay(1000);
          switchOn(rel1); switchOn(rel2);
          delay(3000);
          switchOff(rel1); switchOff(rel2);
          break;
        case 5:
          delay(1000);
          switchOn(rel3); switchOn(rel4);
          delay(3000);
          switchOff(rel3); switchOff(rel4);
          break;
      }
  
      delay(2000);
    }
  }

  void testDataRefresh(SwitchDef sw_InfoPanel_Buzzer, 
                       SwitchDef sw_Relay1_ALARM, SwitchDef sw_Relay2_HEAT_FAN,
                       SwitchDef sw_Relay3_POWER, SwitchDef sw_Relay4) {
    bool buzzWasOn = isSwitchOn(sw_InfoPanel_Buzzer);
    if (buzzWasOn) {
      delay(200);
      switchOff(sw_InfoPanel_Buzzer);
      delay(200);
    }
    bool rel1WasOn = isSwitchOn(sw_Relay1_ALARM);
    bool rel2WasOn = isSwitchOn(sw_Relay2_HEAT_FAN);
    bool rel3WasOn = isSwitchOn(sw_Relay3_POWER);
    bool rel4WasOn = isSwitchOn(sw_Relay4);
  
    switchOn(sw_InfoPanel_Buzzer);
    switchToggleTo(sw_Relay1_ALARM, RS485Server::f1);
    switchToggleTo(sw_Relay2_HEAT_FAN, RS485Server::f2);
    switchToggleTo(sw_Relay3_POWER, RS485Server::f3);
    switchToggleTo(sw_Relay4, RS485Server::f4);
  
    delay(1000);
  
    switchToggleTo(sw_InfoPanel_Buzzer, buzzWasOn);
    switchToggleTo(sw_Relay1_ALARM, rel1WasOn);
    switchToggleTo(sw_Relay2_HEAT_FAN, rel2WasOn);
    switchToggleTo(sw_Relay3_POWER, rel3WasOn);
    switchToggleTo(sw_Relay4, rel4WasOn);
  }
}
