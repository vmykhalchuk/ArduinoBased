#ifndef POWER_CIRCUIT_H
#define POWER_CIRCUIT_H

#include <Arduino.h>
#include "switch_pin.h"

namespace PowerCircuit {

  void init(SwitchDef &sw_Relay3_POWER, SwitchDef &sw_Heater_TRIACs);

  void updateRemoteFlags(bool powerRequest, bool heatRequest, bool fanRequest, bool alarm);

  // Units:
  //   - Relay for Contactor
  //   - TRIACs
  //   - Relay for Heating FAN
  //   - Relay for Alarm (can be only turned on when Contactor Relay is OFF)

  // States:
  //   - OFF - state where everything is Off, however waiting for PowerOnRequest
  //   - BEFORE_ON_ALARM_DELAY - Sounding Alarm for 3 sec to notify that power is gonna be switched on
  //   - ON
  //   - ??? ON_BEFORE_HEATING_DELAY - Waiting for 2 sec before enabling TRIACs to prevent failures (used only if Heat was requested soon after power was turned on)
  //   - BEFORE_OFF_DELAY - do not turn Contactor off without 2s delay after TRIACs disabled
  void loop();
}
#endif
