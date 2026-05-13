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

/*
 * FIXME/TODO List
 *  
 *  - [CRITICAL] Add Error handling and delayed back to normal state (switch pin to Z and delay for 5sec)
 *  - [CRITICAL] Add Data validation (CRC or Inverted data) (Write back XOR of two bits (8bits=>4bits)) Only when it returns intact to Transmitter - TX will discard cached flags as sent.
 *                    We do Tx->RX then RX->TX - TX can received partial CRC from RX to validate if correct error were sent, then assume transmission was good
 *  
 *  - [HIGH] FIX writing error handling - when fails - recover cleared errorFlags
 *  - [HIGH][PERFORMANCE] Replace Switch with method pointer to improve perfromance (especially important for wait loops) (best will be simple IJMP (also allign commands into same block sizes, so quick multiplication will lead to required case) to save on RET)
 *  
 *  - [HIGH] Document:
                - line states
                - ownership
                - timing
                - bit encoding
                - startup handshake
                - collision rules
                - timeout behavior
                - recovery behavior
 *  
 *  - Fix registerError function - see its comments
 *  - make it more flexible in configuration (not only PORTD as a debug pin)
 *  - add option to disable extra protocol checks - to save programm space
 */

namespace pinkyvolt::debug {

  uint8_t val = *(volatile uint8_t*)&timer0_overflow_count;
  
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

      static void readTime0Assembly() {
        uint8_t timerVal;
        asm volatile ("in %0, %1" : "=r" (timerVal) : "I" (_SFR_IO_ADDR(TCNT0)));
      }

  };

}

#endif
