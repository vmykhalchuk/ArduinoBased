#include "input_button.h"

namespace InputButton {

  void loop(Def &def) {
    Internal &_ctx = def._ctx;

    unsigned long _ms = millis();
    switch (_ctx.smState) {
      case NOT_INITIALIZED:
        pinMode(def.pinNo, def.enablePullup ? INPUT_PULLUP : INPUT);
        if (def.enablePullup) digitalWrite(def.pinNo, HIGH);
        _ctx.btnState = digitalRead(def.pinNo);
        _ctx.lastStateChangedTmstmp = _ms;
        _ctx.timerMark = _ms;
        _ctx.smState = DEBOUNCE_WAITING;
      break;
      case IDLE:
        if (digitalRead(def.pinNo) != _ctx.btnState) {
          _ctx.timerMark = _ms;
          _ctx.smState = DEBOUNCE_WAITING;
        }
      break;
      case DEBOUNCE_WAITING:
        if (_ms - _ctx.timerMark >= DEBOUNCE_THRESHOLD_MS) {
          if (_ctx.btnState != digitalRead(def.pinNo)) {
            _ctx.btnState = !_ctx.btnState;
            _ctx.lastStateChangedTmstmp = _ms;

            if (_ctx.btnState) {
              _ctx.wasPressed = true;
              _ctx.wasLongPressed = false;
            } else {
              _ctx.wasReleased = true;
            }
          }
          _ctx.smState = IDLE;
        }
      break;
      default:
        _ctx.smState = ERROR;
      break;
    }
  }

  bool isPressed(Def &def) {
    return def._ctx.btnState == def.isActiveHigh;
  }

  bool isLongPressed(Def &def) {
    return def._ctx.btnState == def.isActiveHigh && (millis() - def._ctx.lastStateChangedTmstmp > LONG_PRESS_DURATION_MS);
  }

  bool wasPressed(Def &def) {
    if (def._ctx.wasPressed) {
      def._ctx.wasPressed = false;
      return true;
    }
    return false;
  }

  bool wasLongPressed(Def &def) {
    if (def._ctx.wasLongPressed) {
      return false;
    }
    bool longPressDetected = isLongPressed(def);
    if (longPressDetected) {
      def._ctx.wasLongPressed = true;
      return true;
    }
    return false;
  }

  bool wasReleased(Def &def) {
    if (def._ctx.wasReleased) {
      def._ctx.wasReleased = false;
      return true;
    }
    return false;
  }

  bool hasStateChanged(Def &def) {
    return wasPressed(def) || wasReleased(def);
  }

  bool isError(Def &def) {
    return def._ctx.smState == ERROR;
  }

}
