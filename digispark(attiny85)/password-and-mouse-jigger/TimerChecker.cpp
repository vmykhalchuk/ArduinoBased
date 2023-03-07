#include "TimerChecker.h"

void TimerChecker::restart() {
  this->tmstmp = Kingdom::millisShort;
}

void TimerChecker::restart(uint16_t timeoutMs) {
  this->timeoutMs = timeoutMs;
  if (timeoutMs == 0) return;
  this->tmstmp = Kingdom::millisShort;
}

void TimerChecker::restartPrecisely() {
  this->tmstmp += timeoutMs;
}

void TimerChecker::stop() {
  timeoutMs = 0;
}
