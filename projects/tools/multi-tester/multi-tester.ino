#include <Arduino.h>
#include "clock.h"
#include "input_button.h"
#include "kh2441ef.h"

InputButton::Def btnMain = { .pinNo = 3, .isActiveHigh = false, .enablePullup = true };

uint16_t tickStartMs;
void setup() {
  tickStartMs = ClockLR::now;
}

uint8_t i = 0;
void loop() {
  ClockLR::tick();
  InputButton::tick(btnMain);
  KH2441EF::displayLoopAsync();
  if (ClockLR::isElapsed(tickStartMs, 1000)) {
    tickStartMs = ClockLR::now;
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
