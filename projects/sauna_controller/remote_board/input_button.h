#ifndef INPUT_BUTTON_H
#define INPUT_BUTTON_H

#include <Arduino.h>


namespace InputButton {

  static const unsigned int DEBOUNCE_THRESHOLD_MS = 50;
  static const unsigned int LONG_PRESS_DURATION_MS = 1000; // FIXME Make it configurable

  enum SMState { NOT_INITIALIZED, IDLE, DEBOUNCE_WAITING, ERROR };
  
  struct Internal {
    bool btnState = false; // FIXME Put all bool as flags to optimize space
    bool wasPressed = false;
    bool wasReleased = false;
    bool wasLongPressed = false;
    unsigned long lastStateChangedTmstmp = 0;
    unsigned long timerMark = 0;
    SMState smState = NOT_INITIALIZED;
  };

  struct Def {
    const int pinNo;
    const bool isActiveHigh;
    const bool enablePullup;
    Internal _ctx;
  };

  void loop(Def &def);

  bool isPressed(Def &def);
  bool isLongPressed(Def &def);

  // was/has functions return `true` only once - when calling second time and button is still pressed/released - it returns `false`
  bool wasPressed(Def &def);
  bool wasLongPressed(Def &def);
  bool wasReleased(Def &def);

  bool hasStateChanged(Def &def);

  //FIXME Implement. Add lastPressedTmstmp into Internal. Add lastPressedLengthMs - to track if last pressed was short or long (If first was a long press - then no double click)
  //bool isDoubleClicked();

  //FIXME Add auto-repeat into wasPressed()

  bool isError(Def &def);
}

#endif
