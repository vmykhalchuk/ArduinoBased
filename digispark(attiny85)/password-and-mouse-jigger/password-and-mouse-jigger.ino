const int pinBtn1 = 2; // PB2
const int pinLed = 1;  // PB1

#include "DigiKeyboard.h"

#include "TimerChecker.h"
TimerChecker timerCheckerForClicksCounter(501);

#include "BtnPressStateMachine.h"
BtnPressStateMachine btn1Sm(pinBtn1, HIGH);

void setup() {
  pinMode(pinBtn1, INPUT_PULLUP);
  pinMode(pinLed, OUTPUT);
}

void loop() {
  loopD2();
}

int loopD1_i = 0;
void loopD1() {
  delay(200);
  if (digitalRead(pinBtn1) == LOW) {
    digitalWrite(pinLed, HIGH);
  } else {
    digitalWrite(pinLed, LOW);
  }

  if (loopD1_i++ >= 10) {
    DigiKeyboard.sendKeyStroke(KEY_ARROW_LEFT);
    loopD1_i = 0;
  }
}

int nClick = 0;

void loopD2() {
  btn1Sm.loop();

  int resOnClick = 0; // 0 - no click; 1,2,3 - single,double,triple click; -5 - too many clicks; 10 - long click; -10 - error on long click

  bool timerTimedOut = timerCheckerForClicksCounter.isTimedOut();
  if (timerTimedOut) {
    if (nClick > 0) {
      resOnClick = (nClick > 3) ? -5 : nClick;
    }
    nClick = 0;
    timerCheckerForClicksCounter.stop();
  }

  if (btn1Sm.isResult()) {
    if (btn1Sm.takeResultMs() >= 1000) {
      // long click, fail if nClick != 0
      resOnClick = (nClick == 0) ? 10 : -10;
      nClick = 0;
    } else if (!timerTimedOut) {
      timerCheckerForClicksCounter.restart(500);
      nClick++;
    } else {
      resOnClick = -99; // should not occur!
    }
  }

  if (resOnClick != 0) {
    DigiKeyboard.sendKeyStroke(0);
    DigiKeyboard.print(" {");
    DigiKeyboard.print(resOnClick);
    DigiKeyboard.print("} ");
  }
}