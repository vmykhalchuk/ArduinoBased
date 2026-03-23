#include "tests.h"

namespace Tests {

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

  //                sw_InfoPanel_Buzzer, sw_Relay1_ALARM, sw_Relay2_HEAT_FAN, sw_Relay3_POWER, sw_Relay4
  void testDataRefresh(SwitchDef buzzer, SwitchDef rel1, SwitchDef rel2, SwitchDef rel3, SwitchDef rel4) {
    bool buzzWasOn = isSwitchOn(buzzer);
    if (buzzWasOn) {
      delay(200);
      switchOff(buzzer);
      delay(200);
    }
    bool rel1WasOn = isSwitchOn(rel1);
    bool rel2WasOn = isSwitchOn(rel2);
    bool rel3WasOn = isSwitchOn(rel3);
    bool rel4WasOn = isSwitchOn(rel4);
  
    switchOn(buzzer);
    if (RS485Server::f1) switchOn(rel1); else switchOff(rel1);
    if (RS485Server::f2) switchOn(rel2); else switchOff(rel2);
    if (RS485Server::f3) switchOn(rel3); else switchOff(rel3);
    if (RS485Server::f4) switchOn(rel4); else switchOff(rel4);
  
    delay(1000);
  
    if (buzzWasOn) switchOn(buzzer); else switchOff(buzzer);
    if (rel1WasOn) switchOn(rel1); else switchOff(rel1);
    if (rel2WasOn) switchOn(rel2); else switchOff(rel2);
    if (rel3WasOn) switchOn(rel3); else switchOff(rel3);
    if (rel4WasOn) switchOn(rel4); else switchOff(rel4);
  }
}
