#include "BtnPressStateMachine.h"

uint8_t BtnPressStateMachine::buttonIdGen = 0;

void BtnPressStateMachine::loop() {
  bool btnCurrState;
  switch (state) {
    case ZERO:
      btnCurrState = digitalRead(pinBtn);
      if (btnCurrState != defaultBtnState) {
        Serial.print(F("btn #")); Serial.print(buttonId); Serial.print(F(" pressed: "));
        Serial.println(btnCurrState);
        timerChecker.restart();
        v = 0;
        result = 0;
        CSTATE(PRESSED);
      }
      break;

    case PRESSED:
      if (timerChecker.isTimedOut()) {
        v++;
        timerChecker.restart();
        btnCurrState = digitalRead(pinBtn);
        if (btnCurrState == defaultBtnState) {
          CSTATE(RELEASED);
        }
      }
      break;

    case RELEASED:
      setResult(v); // set result to be taken by main SM
      Serial.print(F("btn #")); Serial.print(buttonId); Serial.print(F(" released: "));
      Serial.print(btnCurrState);
      Serial.print(F("; result: ")); Serial.println(this->result);
      timerChecker.restart();
      CSTATE(COOLDOWN);
      break;

    case COOLDOWN:
      // waiting for result to be taken by main SM and minimum delay to pass (debouncing to occur)
      if (!isResult() && timerChecker.isTimedOut()) {
        _reset();
      }
      break;

    default:
      // FIXME handle this error somehow!
      Serial.print(F("ERRBtnPrssSM:")); Serial.println(state);
      _reset();
  };
}
