#ifndef PINKYVOLT_DEBUG_H
#define PINKYVOLT_DEBUG_H

#include <Arduino.h>
#include <wiring.c>
// taps directly into Arduino millis implementation
extern volatile long unsigned int timer0_overflow_count;

#include "pinkyvolt_debug_rx.hpp"
#include "pinkyvolt_debug_tx.hpp"

/*
 * Debug tool used to track and report errors in PinkyVolt Projects.
 * It uses single wire to communicate Transmitter with Receiver.
 */

namespace pinkyvolt::debug {

  uint8_t val = *(volatile uint8_t*)&timer0_overflow_count;

  // SPEED_MODE - optimized for speed - additional cost in Program memory
  template<bool SPEED_MODE>
  class StateFlags final {
    private:
      StateFlags() {};
      static uint8_t _f;

      static constexpr uint8_t _ERROR = 1<<0;
      static constexpr uint8_t _ERROR_MASK = ~_ERROR;
    public:

      bool isError() {
        return _f & _ERROR;
      }
      void setError() {
        _f |= _ERROR;
      }
      void clearError() {
        if (SPEED_MODE) {
          _f &= _ERROR_MASK;
        } else {
          _f &= ~_ERROR;
        }
      }
    
  };
  
  class Util final {
    public:

      // exampleHowToReadOverflow
      static uint32_t get_overflow_count() {
        uint32_t count;
        uint8_t oldSREG = SREG;  // Save interrupt state
        cli();                   // Disable interrupts
        count = timer0_overflow_count;
        SREG = oldSREG;          // Restore interrupt state
        return count;
      }

      static void checkTimer0Overflow() {
        if (TIFR0 & (1 << TOV0)) {
          // Timer0 just overflowed
          TIFR0 = (1 << TOV0); // Clear the flag (TOV0), however where bit is 0 - that flag will stay untouched
        }
      }

      static uint8_t readTimer0() {
        return TCNT0;
      }

      static void readTimer0Assembly() {
        uint8_t timerVal;
        asm volatile ("in %0, %1" : "=r" (timerVal) : "I" (_SFR_IO_ADDR(TCNT0)));
      }

  };

}

#endif
