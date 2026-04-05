#ifndef INFO_PANEL_H
#define INFO_PANEL_H

#include <Arduino.h>
#include "switch_pin.h"

namespace InfoPanel {

  void init(SwitchDef &sw_Led1, SwitchDef &sw_Buzzer);

  void setCommunicationError();
  void clearCommunicationError();

    // FIXME Sound error with buzzer, no need to start Alarm
    // We can display set of errors by using approach:
    //   three beeps/blinks as a start
    //     for every bit if it is set - two blinks; if clear - one blink
    //     2s delay between blinks to make it easier to read
    //   long delay before repeating (10s)
    //
    //  when new error occurs - make 6 beeps and wait for five seconds before displaying error
  // setCode/clearCode - f represents bit No to set/clear
  void setWarningCode(uint8_t f);
  void clearWarningCode(uint8_t f);

  void loop();
}

#endif
