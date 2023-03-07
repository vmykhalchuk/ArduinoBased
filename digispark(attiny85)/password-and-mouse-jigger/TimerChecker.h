// FIXME This must be a part of my library!!!
// This is a copy of same file in "arduino-sketch-war-comm-system-2"
#ifndef TimerChecker_h
#define TimerChecker_h

#include "Arduino.h"
#include "Kingdom.h"

class TimerChecker {
  private:
    uint16_t timeoutMs = 0;
    uint16_t tmstmp = 0;
  public:
    // 0 if stopped
    TimerChecker(uint16_t timeoutMs) {
      restart(timeoutMs);
    };
    TimerChecker() { restart(0); };

    inline bool isTimedOut() __attribute__((always_inline)) {
      if (timeoutMs == 0) return false;
      return (Kingdom::millisShort - tmstmp) >= timeoutMs;
    };

    void restart();
    // 0 to make it stopped and never run
    void restart(uint16_t timeoutMs);
    void restartPrecisely(); // special method to count time precisely. avoid using small timeouts
    void stop();
};

#endif