#include "power_circuit.h"

namespace PowerCircuit {

  SwitchDef *_sw_Relay3_POWER = nullptr, *_sw_Heater_TRIACs = NULL;
  bool _isInitialized = false;

  void init(SwitchDef &sw_Relay3_POWER, SwitchDef &sw_Heater_TRIACs) {
    _sw_Relay3_POWER = &sw_Relay3_POWER;
    _sw_Heater_TRIACs = &sw_Heater_TRIACs;
    _isInitialized = true;
  }


  /*
  enum SystemPowerState {PS_ZERO, PS_OFF, PS_POWERING_ON_DELAY, PS_ON};
  SystemPowerState _sysPowerState = PS_ZERO;
  unsigned long _sysPowerTmstmp = 0;
  
  void loopSysPow() {
    switch (_sysPowerState) {
      case PS_ZERO:
        if (!powerOnRequest) _sysPowerState = PS_OFF;
        break;
      case PS_OFF:
        if (powerOnRequest) {
          if (fireAlarm) {
            switchOff(*sw_Relay3_POWER);
            switchOff(*sw_Relay2_HEAT_FAN);
            switchOn(*sw_Relay1_ALARM);
            _sysPowerState = PS_ZERO;
          } else {
            // toggle Alarm for 3 sec to test it works!
            switchOff(*sw_Relay3_POWER);
            switchOn(*sw_Relay1_ALARM);
            _sysPowerTmstmp = millis();
            _sysPowerState = PS_POWERING_ON_DELAY;
          }
        }
        break;
      break;
      case PS_POWERING_ON_DELAY:
        if (_sysPowerTmstmp - millis() > 3000) {
          switchOff(*sw_Relay1_ALARM);
          switchOn(*sw_Relay3_POWER);
          _sysPowerState = PS_ON;
        }
      break;
      case PS_ON:
      break;
    }
  }
  */

}
