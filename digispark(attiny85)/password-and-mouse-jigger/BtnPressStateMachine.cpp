#include "BtnPressStateMachine.h"

uint8_t BtnPressStateMachine::buttonIdGen = 0;

void BtnPressStateMachine::loop() {
  bool btnCurrState;
  switch (state) {
    case ZERO:
      btnCurrState = digitalRead(pinBtn);
      if (btnCurrState != defaultBtnState) {
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
      _reset();
  };
}
