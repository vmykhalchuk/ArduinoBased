#include <Arduino.h>
#include "clock.h"
#include "input_button.h"
#include "kh2441ef.h"

InputButton::Def btnMain = { .pinNo = 3, .isActiveHigh = false, .enablePullup = true };

uint8_t progNo = 0;
uint16_t mainStartMs;

void setup() {
  mainStartMs = ClockLR::now;
  while (!ClockLR::isElapsed(mainStartMs, InputButton::LONG_PRESS_DURATION_MS << 1)) {
    ClockLR::tick();
    InputButton::tick(btnMain);
    if (InputButton::isLongPressed(btnMain)) {
      // enter select program mode
      selectProgramMode();
    }
  };
  mainStartMs = ClockLR::now;
}

void selectProgramMode() {
  uint16_t startMs = ClockLR::now;
  bool isOn = false;
  uint8_t state = 0; // 0 - waiting for long press to depress
  while (true) {
    ClockLR::tick();
    InputButton::tick(btnMain);
    KH2441EF::tick();
    if (ClockLR::isElapsed(startMs, 500)) {
      // transition screen
      KH2441EF::setDisplayBuf(0, 0, 0x13, isOn ? progNo : 0x10, false);
      isOn = !isOn;
    }
    bool _exit = false;
    switch (state) {
      case 0:
        if (!InputButton::isLongPressed(btnMain)) {
          state = 1;
          InputButton::wasPressed(btnMain); // erase wasPressed flag
        }
      break;
      case 1:
        if (InputButton::wasPressed(btnMain)) {
          progNo++;
          if (progNo == 5) progNo = 0;
        }
        if (InputButton::isLongPressed(btnMain)) _exit = true;
      break;
    }
    if (_exit) break;
  }
}

uint8_t i = 0;
void loop() {
  ClockLR::tick();
  InputButton::tick(btnMain);
  KH2441EF::tick();
  
  switch(progNo) {
    case 0:
      tickProg0();
      break;
    case 1:
      tickProg1();
      break;
    default:
      KH2441EF::setDisplayBufToErrorMsg();
  }
}

void tickProg0() {
  if (ClockLR::isElapsed(mainStartMs, 1000)) {
    mainStartMs = ClockLR::now;
    if (i < 10) {
      if (i%2 == 0) KH2441EF::setDisplayBufToErrorMsg();
      else KH2441EF::clearDisplayBuf();
    } else {
      KH2441EF::clearDisplayBuf();
      KH2441EF::setDisplayBuf(1,2,3,4,i%2);
    }
    i++;
  }
}

void tickProg1() {
  if (ClockLR::isElapsed(mainStartMs, 100)) {
    uint16_t v = analogRead(A0);
    KH2441EF::setDisplayBufToInt(v);
  }
}
