#include <Arduino.h>
#include "kh2441ef.h"

uint16_t tickStartMs;
void setup() {
  tickStartMs = millis();
}

uint8_t i = 0;
void loop() {
  uint16_t _m = millis();
  KH2441EF::displayLoopAsync(_m);
  if (_m - tickStartMs > 1000) {
    tickStartMs = _m;
    if (i < 10) {
      if (i%2 == 0) KH2441EF::setDisplayBufToErrorMsg();
      else KH2441EF::clearDisplayBuf();
    } else {
      KH2441EF::clearDisplayBuf();
      KH2441EF::setDisplayBuf(1,2,3,4,i%2);
    }
  }
}
