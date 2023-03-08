const int pinBtn1 = 2; // PB2
const int pinLed = 1;  // PB1

const int LONG_CLICK = 5;

#include "DigiKeyboard.h"
#include "DigiMouse.h"

#include "TimerChecker.h"
TimerChecker timerCheckerForClicksCounter(501);
TimerChecker timerCheckerForMouseJigger(500);
TimerChecker timerCheckerForMouseAndKeyboardUsbUpdate(20);

#include "BtnPressStateMachine.h"
BtnPressStateMachine btn1Sm(pinBtn1, HIGH);

void setup() {
  pinMode(pinBtn1, INPUT_PULLUP);
  pinMode(pinLed, OUTPUT);

  DigiMouse.init(); // call init to enumerate

  timerCheckerForMouseAndKeyboardUsbUpdate.restart();
}

void loop() {
  loopD2();
}

int loopD1_i = 0;
void loopD1() {
  delayN25ms(2*4);
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
bool mouseJiggering = false;
void loopD2() {
  btn1Sm.loop();
  if (timerCheckerForMouseAndKeyboardUsbUpdate.isTimedOut()) {
    DigiKeyboard.update();
    DigiMouse.update();
    timerCheckerForMouseAndKeyboardUsbUpdate.restart();
  }

  int resOnClick = 0; // 0 - no click; 1,2,3 - single,double,triple click; -5 - too many clicks; 5 - long click; -10 - error on long click

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
    handleClicks_debug(resOnClick);
    handleClicks(resOnClick);
  }

  if (mouseJiggering) {
    if (timerCheckerForMouseJigger.isTimedOut()) {
      DigiMouse.moveY(5);
      timerCheckerForMouseJigger.restart();
    }
  }
}

// 1,2,3 or 5 (long click)
void handleClicks_debug(int nClicks) {
  blinkLedNTimes((nClicks > 0 && nClicks <= 3) ? nClicks : 5);
}

// 1,2,3 or 5 (long click)
void handleClicks(int nClicks) {
  if (nClicks == LONG_CLICK) {
    DigiKeyboard.sendKeyStroke(0);
    DigiKeyboard.print("Gm0dFL_23_Eq9");
    DigiKeyboard.sendKeyStroke(KEY_ENTER);
  } else if (nClicks == 2) {
    mouseJiggering = !mouseJiggering;
    timerCheckerForMouseJigger.restart();
  }
}

void blinkLedNTimes(uint8_t blinkTimes) {
  for (int i = 0; i < blinkTimes; i++) {
    digitalWrite(pinLed, HIGH);
    delayN25ms(3*4);
    digitalWrite(pinLed, LOW);
    delayN25ms(5*4);
  }
}

void delayN25ms(int n25ms) {
  for (int i = 0; i < n25ms; i++) {
    DigiKeyboard.delay(25);
    DigiMouse.update();
  }
}