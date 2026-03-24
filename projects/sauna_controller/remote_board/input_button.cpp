#include "input_button.h"

namespace InputButton {

  void loopFor(Def &def) {
    Internal _ = def._;
    if (_.v2 != -2) {
      // Default initialization doesn't work!!! error!
      _.smState = ERROR;
    }
    _.v2 = 0;

    unsigned long _ms = millis();
    switch (_.smState) {
      case NOT_INITIALIZED:
        pinMode(def.pinNo, INPUT);
        _.btnState = digitalRead(def.pinNo);
        _.lastStateChangedTmstmp = _ms;
        _.timerMark = _ms;
        _.smState = DEBOUNCE_WAITING;
      break;
      case IDLE:
        if (digitalRead(def.pinNo) != _.btnState) {
          _.timerMark = _ms;
          _.smState = DEBOUNCE_WAITING;
        }
      break;
      case DEBOUNCE_WAITING:
        if (_ms - _.timerMark >= DEBOUNCE_THRESHOLD_MS) {
          if (_.btnState != digitalRead(def.pinNo)) {
            _.btnState = !_.btnState;
            _.lastStateChangedTmstmp = _ms;
          }
          _.smState = IDLE;
        }
      break;
    }
  }

  bool isPressed(Def &def) {
    if (def._.btnState == def.isActiveHigh) {
      return true;
    } else {
      return false;
    }
  }

  bool isError(Def &def) {
    return def._.smState == ERROR;
  }
}
