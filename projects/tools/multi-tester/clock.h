#ifndef CLOCK_H
#define CLOCK_H

#include <Arduino.h>

class ClockHR {
  public:
    static uint32_t now; // Static variable shared by everyone
    static void tick() {
      now = millis();
    }
    static uint32_t millis() {
      tick();
      return now;
    }
    static bool isElapsed(uint32_t since, uint16_t intervalMs) {
      return now - since >= intervalMs;
    }
};

class ClockLR {
  public:
    static uint16_t now; // low resolution version
    static void tick() {
      ClockHR::tick();
      now = ClockHR::now;
    }
    static uint16_t millis() {
      tick();
      return now;
    }
    static bool isElapsed(uint16_t since, uint16_t intervalMs) {
      return now - since >= intervalMs;
    }
    // `lowerIntervalMs` must be << `intervalMs` and the bigger the gap - the better - less time wasted to update `since`
    static void preventTimerOverrun(uint16_t &since, uint16_t intervalMs, uint16_t lowerIntervalMs) {
      if (isElapsed(since, intervalMs)) {
        since = now - lowerIntervalMs;
      }
    }
};

#endif
