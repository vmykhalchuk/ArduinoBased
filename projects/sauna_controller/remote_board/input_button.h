#ifndef INPUT_BUTTON_H
#define INPUT_BUTTON_H

#include <Arduino.h>


namespace InputButton {

  static const unsigned int DEBOUNCE_THRESHOLD_MS = 300;//50;

  enum SMState { NOT_INITIALIZED, IDLE, DEBOUNCE_WAITING, ERROR };
  
  struct Internal {
    bool btnState = false;
    unsigned long lastStateChangedTmstmp = 0;
    int v2 = -2;
    unsigned long timerMark = 0;
    SMState smState = NOT_INITIALIZED;
  };

  struct Def {
    int pinNo;
    bool isActiveHigh;
    bool enablePoolup;
    Internal _;
  };

  void loopFor(Def &def);

  bool isPressed(Def &def);

  bool isError(Def &def);

  bool isIdleState(Def &def);
}

#endif
