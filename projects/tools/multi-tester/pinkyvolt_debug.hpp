#ifndef PINKYVOLT_DEBUG_H
#define PINKYVOLT_DEBUG_H

#include <wiring.c>
// taps directly into Arduino millis implementation
extern volatile long unsigned int timer0_overflow_count;

#include <Arduino.h>
#include <util/atomic.h>

#include "clock.h"

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

namespace pinkyvolt::debug::tx {
  
  enum SMState {
              WAITING4CONNECTION, WAITING4CONNECTION_S1, WAITING4CONNECTION_S2, WAITING4CONNECTION_S3, WAITING4CONNECTION_S4, WAITING4CONNECTION_S5,
              WRITING, WRITING_FLAG_S0, WRITING_FLAG_S1, WRITING_FLAG_S2, WRITING_FLAG_S3, WRITING_ERROR,
              READING, READING_BIT_S0, READING_BIT_S1, READING_BIT_S2, READING_BIT_S3, READING_BIT_S4,
              ERROR,
              SKIP_CURRENT_CYCLE, SKIP_FULL_CYCLE, SKIP_TWO_FULL_CYCLES, LONG_WAIT };

  template <uint8_t PORTD_PIN>
  class OneWireErrorTransmitter final {
    
    private:
    
      static_assert((PORTD_PIN <= 7), "PORTD_PIN must be within 0..7 range!");

      static constexpr uint8_t _MASK = 1<<PORTD_PIN;

      enum StateGroup {
        _G_CRIT = 0x10,
        _G_HANDSHAKE = 0x20,
        _G_WRITING = 0x30,
        _G_READING = 0x40
      };
      enum FSMState {
        _CRIT__WAITING_FOR_HANDSHAKE        = _G_CRIT | 0,
        _CRIT__SKIP_CURRENT_CYCLE           = _G_CRIT | 1,
        _CRIT__SKIP_FULL_CYCLE              = _G_CRIT | 2,
        _CRIT__SKIP_TWO_FULL_CYCLES         = _G_CRIT | 3,
        _CRIT__LONG_WAIT                    = _G_CRIT | 4,

        _HS__S1                             = _G_HANDSHAKE | 1,
        _HS__S2                             = _G_HANDSHAKE | 2,
        _HS__S3                             = _G_HANDSHAKE | 3,
        _HS__S4                             = _G_HANDSHAKE | 4,
        _HS__S5                             = _G_HANDSHAKE | 5,

        _WR_S0                              = _G_WRITING | 0,
        _WR_S1                              = _G_WRITING | 1,
        _WR_S2                              = _G_WRITING | 2,
        _WR_S3                              = _G_WRITING | 3,
        _WR_ERR                             = _G_WRITING | 4,
      };
      
      SMState _state = ERROR;

      // Set D<PORTD_PIN> To Output (1 cycle)
      inline void setDPinToOutput() { asm volatile ("sbi %0, %1" : : "I" (_SFR_IO_ADDR(DDRD)), "I" (PORTD_PIN) : "memory"); }

      // Set D<PORTD_PIN> To Output (1 cycle)
      inline void setDPinToInput() { asm volatile ("cbi %0, %1" : : "I" (_SFR_IO_ADDR(DDRD)), "I" (PORTD_PIN) : "memory"); }

      // Set D<PORTD_PIN> HIGH (1 cycle)
      inline void setDPinToHigh() { asm volatile ("sbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTD)), "I" (PORTD_PIN) : "memory"); }

      // Set D<PORTD_PIN> LOW (1 cycle)
      inline void setDPinToLow() { asm volatile ("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTD)), "I" (PORTD_PIN) : "memory"); }

      // Toggle D<PORTD_PIN> (1 cycle)
      // Writing a 1 to the PIN register toggles the PORT bit
      inline void toggleDPinState() { asm volatile ("sbi %0, %1" : : "I" (_SFR_IO_ADDR(PIND)), "I" (PORTD_PIN) : "memory"); }

      inline bool isDPinHigh() { return PIND & _MASK; }

      inline bool isDPinLow() { return !(PIND & _MASK); }

      enum TXMode {
        __Z,
        _PU,
        _AL,
        _AH,
        TXModeCount
      };
      
      TXMode _currTXMode = __Z;
                                      //   A A P _
                                      //  _H_L_U_Z
      static constexpr uint8_t __Z2__ = 0B00110100;
      static constexpr uint8_t _PU2__ = 0B11000001;
      static constexpr uint8_t _AL2__ = 0B01000010;
      static constexpr uint8_t _AH2__ = 0B00011000;
                                      //=> 2bit opCodes:
                                      //        00->not allowed(error)    01->toggle PIN
                                      //        10->set as input          11->set as output
      static constexpr uint8_t _TRANSITION_MAP[] = {__Z2__, _PU2__, _AL2__, _AH2__};

      __attribute__((always_inline))
      inline bool transitionTX(TXMode changeTo) {
        static_assert(TXModeCount==4, "TXModeCount must be = 4!");

        uint8_t _trnst = _TRANSITION_MAP[(uint8_t)_currTXMode];
        uint8_t opCode = _trnst >> ((uint8_t)changeTo<<1);
        opCode &= 0x3;

        switch (opCode) {
          case 0: // not allowed (error)
            return false;
          case 1: // toggle pin
            toggleDPinState();
            break;
          case 2: // set as input
            setDPinToInput();
            break;
          case 3: // set as output
            setDPinToOutput();
            break;
        }
        return true;
      }

      SMState _skipCycleReturnState = ERROR;
      uint8_t _skipCycleTmr0 = 0;
      uint8_t _skipCycleTmr0p1 = 0;
      uint8_t _skipCycleCounter = 0;

      // assures to skip current cycle (as a minimum)
      // FIXME Looks like there is no use for this function - remove it
      inline void skipCurrentCycle(SMState retState) {
        _skipCycleReturnState = retState;
        _state = SKIP_CURRENT_CYCLE;

        asm volatile("" ::: "memory");
        _skipCycleTmr0 = TCNT0;
      }

      // assures to skip full cycle (as a minimum 4uS for Receiver to react)
      inline void skipFullCycle(SMState retState) {
        _skipCycleReturnState = retState;
        _state = SKIP_FULL_CYCLE;

        asm volatile("" ::: "memory");
        _skipCycleTmr0p1 = _skipCycleTmr0 = TCNT0;
        _skipCycleTmr0p1++;
      }

      // assures to skip two full cycle (as a minimum 8uS for Receiver to react)
      inline void skipTwoFullCycles(SMState retState) {
        _skipCycleReturnState = retState;
        _skipCycleCounter = 0;
        _state = SKIP_TWO_FULL_CYCLES;

        asm volatile("" ::: "memory");
        _skipCycleTmr0 = TCNT0;
      }


      volatile uint8_t _errorFlags = 0; // FIXME Rename to data (avoid error in name, make separate class to register error flags)

      uint8_t _writingErrorFlags = 0;   // FIXME Rename to _writingDataBits
      uint8_t _writingFlag = 0;         // FIXME Rename to _writingBit

      uint8_t _readingData = 0;
      uint8_t _readingBit = 0;
  
    public:

      void registerError(uint8_t errorFlag) {
        // TODO Make this operation atomic
        //     1) move errorFlag to GPIOR0 <not GPIOR1 nor GPIOR2 since they are in extended area (above 0xFF)>
        //    +2) Suspend interrupts
        //     3) ignore it - since we are setting bit - set after set => same result
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
          _errorFlags |= 1 << (errorFlag & 0x07);
        }
      }

      void registerErrorsInBulk(uint8_t errorFlagsSet) {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
          // FIXME Limit by the number of flags supported (if less than 8)
          _errorFlags |= errorFlagsSet;
        }
      }

      void setup() {
        {
        setDPinToHigh();
        setDPinToInput();
        _currTXMode = _PU;
        }
        skipFullCycle(WAITING4CONNECTION);
      }
  
      void tick() {
        bool isLow = isDPinLow();
        switch (_state) {
          case WAITING4CONNECTION:
            // wait for M on line
            if (isLow) {                      // Line == M (PD+PU)
              skipFullCycle(WAITING4CONNECTION_S1);
            }
            break;
          case WAITING4CONNECTION_S1:
            if (isLow) {                      // Line == M (PD+PU)
              setDPinToLow();                 // OUT=Z; Line=L (PD+Z)
              skipFullCycle(WAITING4CONNECTION_S2);
            } else {
              // this is unexpected - most likely connection is unstable
              _state = ERROR; // FIXME add proper handling
            }
            break;
          case WAITING4CONNECTION_S2:
            if (isLow) {                      // Line == L (PD+Z)
              // Switch to Active High // After this moment Receiver can verify if its Blind or not to M state of line. If Receiver can see HIGH unexpectedly - it will switch to give-up state
              setDPinToHigh();                // OUT=PU; Line=M (PD+PU)
              skipFullCycle(WAITING4CONNECTION_S3);
            } else {
              // this is unexpected - most likely connection is unstable
              _state = ERROR; // FIXME add proper handling
            }
            break;
          case WAITING4CONNECTION_S3:
            if (isLow) {                      // Line == M (PD+PU)
              setDPinToOutput();              // OUT=AH; Line=H (PD+AH)
              skipFullCycle(WAITING4CONNECTION_S4);
            } else {
              // if HIGH - Receiver has disconnected (most likely it failed to determine M state from LOW state)
              _state = ERROR; // FIXME add proper handling
            }
            break;
          case WAITING4CONNECTION_S4:         // Line == H (PD+AH)
            setDPinToInput();                 // OUT=PU; Line=M
            skipFullCycle(WAITING4CONNECTION_S5);
            break;
          case WAITING4CONNECTION_S5:
            if (isLow) {                      // Line == M (PD+PU)
              // FIXME What to do next?
              skipFullCycle(WRITING); // FIXME??? Change it if we do not want to Write first
            } else {
              // something wrong - connection issue
              _state = ERROR; // FIXME add proper handling
            }
            break;

          case WRITING:
            if (isLow) {                      // Line == M (PD+PU)
              _writingErrorFlags = _errorFlags;
              _errorFlags = 0; // reset - if writing fails we update _errorFlags with _writeErrorFlags <= FIXME Implement that logic
              _writingFlag = 0; // start from bit 0
              _state = WRITING_FLAG_S0;
            } else {
              _state = WRITING_ERROR;
            } 
            break;

          case WRITING_FLAG_S0:
            if (isLow) {                      // Line == M (PD+PU)
              uint8_t mask = 1 << _writingFlag;
              if (_writingErrorFlags & mask) {
                // Write 0
                setDPinToLow();               // OUT=Z; Line=L (PD+Z)
                skipFullCycle(WRITING_FLAG_S1);
              } else {
                // Write 1
                setDPinToOutput();            // OUT=AH; Line=H (PD+AH)
                skipFullCycle(WRITING_FLAG_S2);
              }
            } else {
              _state = WRITING_ERROR;
            }
            break;

          case WRITING_FLAG_S1:
            // We were writing 0
            if (isLow) {                      // Line == L (PD+Z)
              setDPinToHigh();                // OUT=PU; Line=M (PD+PU)
              _state = WRITING_FLAG_S3;
            } else {
              _state = WRITING_ERROR;
            }
            break;

          case WRITING_FLAG_S2:
            // We were writing 1              // Line == H (PD+AH)
            setDPinToInput();                 // OUT=PU; Line=M (PD+PU)
            _state = WRITING_FLAG_S3;
            break;

          case WRITING_FLAG_S3:
            _writingFlag++;
            if (_writingFlag <= 7) {
              skipFullCycle(WRITING_FLAG_S0);
            } else {
              // last bit was written - exit
              // FIXME - where to go from here?
              //switchToCycleWait(WAITING4CONNECTION); //FIXME???
              skipFullCycle(READING);//FIXME???
            }
            break;

          case WRITING_ERROR:
            // recover flags back (were not sent to Receiver)
            registerErrorsInBulk(_writingErrorFlags);
            skipFullCycle(ERROR); // FIXME add proper handling
            break;
          

          case READING:
            if (isLow) {                      // Line == M (PD+PU)
              _readingData = 0;
              _readingBit = 0;
              _state = READING_BIT_S0;
            } else {
              _state = ERROR; // FIXME add proper handling
            }
            break;

          case READING_BIT_S0:
            if (isLow) {                      // Line == M (PD+PU)
              setDPinToOutput();              // OUT=AH; Line=H (PD+AH)
              skipFullCycle(READING_BIT_S1);
            } else {
              _state = ERROR; // FIXME add proper handling
            }
            break;
          case READING_BIT_S1:
            if (!isLow) {                     // Line == H (PD+AH)
              setDPinToLow();                 // OUT=AL; Line=L (PD+AL) (No possible action on Receiver side can change Line state, so it is safe to transition here in two steps)
              setDPinToInput();               // OUT= Z; Line=L (PD+Z)
              skipFullCycle(READING_BIT_S2);
            } else {
              _state = ERROR; // FIXME add proper handling
            }
            break;
          case READING_BIT_S2:
            if (isLow) {                      // Line == L (PD+Z)
              setDPinToHigh();                // OUT=PU; Line=M (PD+PU)
              skipFullCycle(READING_BIT_S3);
            } else {
              _state = ERROR; // FIXME add proper handling
            }
            break;
          case READING_BIT_S3:
            if (!isLow) {
              _readingData |= 1 << _readingBit;
            }
            if (isLow) {                    // Line == M (PD+PU)
              setDPinToLow();               // OUT=Z; Line = L (PD+Z)   // FIXME Make Sure Receiver DOESN'T ACT AT THIS M=>L TRANSITION!!!
              setDPinToOutput();            // OUT=AL; Line = L (PD+AL)
              skipFullCycle(READING_BIT_S4);
            } else {                        // Line == H (Z+PU)
              setDPinToOutput();            // OUT=AH; Line = H (Z+AH)
              setDPinToLow();               // OUT=AL; Line = L (Z+AL)
              skipFullCycle(READING_BIT_S4);
            }
            break;
          case READING_BIT_S4:                // Line == L (PD+AL)
            setDPinToInput();                 // OUT=Z; Line = L (PD+Z)
            setDPinToHigh();                  // OUT=PU; Line = M (PD+PU)
            _readingBit++;
            if (_readingBit <= 3) { // we read 4 bits only (configure same for Receiver!)
              skipFullCycle(READING_BIT_S0);
            } else {
              // last bit was read - exit
              // FIXME - where to go from here?
              skipFullCycle(WAITING4CONNECTION); //FIXME???
            }
            break;


          case SKIP_CURRENT_CYCLE:
            if (_skipCycleTmr0 != TCNT0) {
              _state = _skipCycleReturnState;
            }
            break;
          // Assures Receiver has min 4uS time to react. Note: It can be way above that time - all depends on how often tick() is called
          case SKIP_FULL_CYCLE:
            // we first check for +1 cycle, we expect in most cases user sketch will be slow
            if (_skipCycleTmr0p1 != TCNT0) {
              if (_skipCycleTmr0 != TCNT0) {
                _state = _skipCycleReturnState;
              } else {
                // we are still at the beginning - wait more (apparently this is test sketch, checking for speed performance)
              }
            }
            break;
          // Assures Receiver has min 8uS time to react
          case SKIP_TWO_FULL_CYCLES:
            if (TCNT0 - _skipCycleTmr0 > 2) {
              _state = _skipCycleReturnState;
            } else {
              if (++_skipCycleCounter > 20) { // 20 * 10 instructions (less is physically impossible) = 200*62.5nS=12.5uS
                // This is fall-back mechanism
                // With 3 / 256 timer states - the chances are that some rhytmic calls will leave SM in this state for very long
                _state = _skipCycleReturnState;
              }
            }
            break;

          // After Error - perform long wait
          case LONG_WAIT:
            // FIXME Implement it!
            break;

          case ERROR:
            _state = LONG_WAIT; // FIXME Implement it!
            break;
        }
      }
  
  };

}

using ErrorTransmitterD0 = pinkyvolt::debug::tx::OneWireErrorTransmitter<0>;
using ErrorTransmitterD1 = pinkyvolt::debug::tx::OneWireErrorTransmitter<1>;
using ErrorTransmitterD2 = pinkyvolt::debug::tx::OneWireErrorTransmitter<2>;
using ErrorTransmitterD3 = pinkyvolt::debug::tx::OneWireErrorTransmitter<3>;
using ErrorTransmitterD4 = pinkyvolt::debug::tx::OneWireErrorTransmitter<4>;
using ErrorTransmitterD5 = pinkyvolt::debug::tx::OneWireErrorTransmitter<5>;
using ErrorTransmitterD6 = pinkyvolt::debug::tx::OneWireErrorTransmitter<6>;
using ErrorTransmitterD7 = pinkyvolt::debug::tx::OneWireErrorTransmitter<7>;
  
namespace pinkyvolt::debug::rx {

  template <uint8_t PORTD_PIN, uint8_t PORTD_PIN_CMP, uint8_t PORTD_PIN_PD>
  class OneWireErrorReceiver final {
    private:
      static_assert((PORTD_PIN == 2), "PORTD_PIN must be 2");
      static_assert((PORTD_PIN_CMP == 3), "PORTD_PIN must be 3");

      static_assert((PORTD_PIN <= 7), "PORTD_PIN must be within 0..7 range!");
      static_assert((PORTD_PIN_CMP <= 7), "PORTD_PIN_OPAMP must be within 0..7 range!");
      static_assert((PORTD_PIN_PD <= 7), "PORTD_PIN_PD must be within 0..7 range!");

      static_assert((PORTD_PIN != PORTD_PIN_CMP), "PORTD_PIN Must not be same as PORTD_PIN_CMP");
      static_assert((PORTD_PIN != PORTD_PIN_PD), "PORTD_PIN Must not be same as PORTD_PIN_PD");
      static_assert((PORTD_PIN_PD != PORTD_PIN_CMP), "PORTD_PIN_PD Must not be same as PORTD_PIN_CMP");

      static_assert((PORTD_PIN_CMP == PORTD_PIN + 1), "PORTD_PIN_CMP MUST be PORTD_PIN + 1");
      

      inline static void __setDPinToInput()  { asm volatile ("cbi %0, %1" : : "I" (_SFR_IO_ADDR(DDRD)),  "I" (PORTD_PIN) : "memory"); }
      inline static void __setDPinToLow()    { asm volatile ("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTD)), "I" (PORTD_PIN) : "memory"); }

      inline static void __setDPinCmpToInput()  { asm volatile ("cbi %0, %1" : : "I" (_SFR_IO_ADDR(DDRD)),  "I" (PORTD_PIN_CMP) : "memory"); }
      inline static void __setDPinCmpToLow()    { asm volatile ("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTD)), "I" (PORTD_PIN_CMP) : "memory"); }

      inline static void __setDPinPDToOutput() { asm volatile ("sbi %0, %1" : : "I" (_SFR_IO_ADDR(DDRD)),  "I" (PORTD_PIN_PD) : "memory"); }
      inline static void __setDPinPDToInput()  { asm volatile ("cbi %0, %1" : : "I" (_SFR_IO_ADDR(DDRD)),  "I" (PORTD_PIN_PD) : "memory"); }
      inline static void __setDPinPDToLow()    { asm volatile ("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTD)), "I" (PORTD_PIN_PD) : "memory"); }

      enum LineState {
        // NOTE: CMP has 1 when voltage is below its threshold (0.15V) and 0 when above
        M, // PIN_CMP==0 && PIN==0 // Middle (PD+PU)
        H, // PIN_CMP==0 && PIN==1 // High   (PD+AH)|( Z+PU)|( Z+AH)
        L, // PIN_CMP==1 && PIN==0 // Low    (PD+ Z)|(PD+AL)|( Z+AL)
        U, // PIN_CMP==1 && PIN==1 // Unknown (Comparator says voltage is below 0.15V but Pin says that it is HIGH - it contradicts, only Hardware failure can cause this)
        LineStateCount
      };

      struct LineThenState {
        // [NOTE] fields order must adhere to same order as LineState!
        uint8_t _m;
        uint8_t _h;
        uint8_t _l;
        uint8_t _u;
      } __attribute__((packed));
  
      inline static LineState _readLine() {
        uint8_t r = PIND >> PORTD_PIN;
        r &= 0x3;
        return (LineState) r;
      }

      /*
             -=- PROTOCOL -=- 

              Handshake:
              M -> L -> M -> H -> M

              Reading 8 bits
                Read 0:
                L -> M
              
                Read 1:
                H -> M

       */

      inline static void _initializePorts() {
        // Main pin to read (no pull-up)
        __setDPinToInput();
        __setDPinToLow();
        // Cmp Pin to read (no pull-up)
        __setDPinCmpToInput();
        __setDPinCmpToLow();
        // Disable PD
        __setDPinPDToInput();
        __setDPinPDToLow();
      }
      inline static void _activatePD() {
        __setDPinPDToOutput();
      }
      inline static void _deactivatePD() {
        __setDPinPDToInput();
      }

      static constexpr uint8_t _SF_IS_CONNECTED = 1<<0;
      static constexpr uint8_t _SF_IS_ERROR = 1<<1;
      static constexpr uint8_t _SF_FRESH_DATA = 1<<2;
      static constexpr uint8_t _SF_WAS_STALLED = 1<<3; // Communication was stalled // FIXME Add support of this Flag (read function and flag set/clear logic)
      static constexpr uint8_t _SF_WDT = 1<<6; // FSM transitions will clear this flag to let tick() reset timer. If though wdt timer elapses - communication is stale - and will be restarted!
      static constexpr uint8_t _SF_COMMERROR_TIMER_ACTIVE = 1<<7; // when comm error happens, coomunication is supsended and timer counts to attempt restart
      static volatile uint8_t _statusFlags;
      static inline bool __isSFSet(uint8_t sf) {
        bool isSet;
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
          isSet = _statusFlags & sf;
        }
        return isSet;
      }
      static inline void __setSF(uint8_t sf) {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
          _statusFlags |= sf;
        }
      }
      static inline void __clearSF(uint8_t sf) {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
          _statusFlags &= ~sf;
        }
      }

      static volatile uint8_t _receivedData;
      
      static uint8_t _readingDataBuf;
      static uint8_t _readingDataBit;

      static LineThenState _lineTransitions;
      static uint8_t* const _lineTransitionsAsArr;

      static constexpr uint16_t ERROR_COMMUNICATION_TIMEOUT_MS = 5000;
      static uint16_t _commErrorTimer;

      // When no change in signal happens within this amount of time => communication is reset
      static constexpr uint16_t COMMUNICATION_STALL_TIMEOUT_MS = 2000;
      static uint16_t _wdtTimer;
      
      __attribute__((always_inline))
      static inline void __lineExpectsLMH(uint8_t _l, uint8_t _m, uint8_t _h) {
        _lineTransitions._l = _l;
        _lineTransitions._m = _m;
        _lineTransitions._h = _h;
      }

      // FIXME Replace with enum
      static constexpr uint8_t _SM_GROUP_1_CRIT = 0x10;
      static constexpr uint8_t _SM_GROUP_2_HANDSHAKE = 0x20;
      static constexpr uint8_t _SM_GROUP_3_READ = 0x30;
      static constexpr uint8_t _SM_GROUP_4_WRITE = 0x40;
      static constexpr uint8_t _SM_GROUP_5_RARE = 0x50;

      static constexpr uint8_t _SM_NOP                          = 0; // DO NOTHING
      
      static constexpr uint8_t _SM_CRIT__WAITING_FOR_CONNECTION = _SM_GROUP_1_CRIT | 0;
      static constexpr uint8_t _SM_CRIT__COMMERROR              = _SM_GROUP_1_CRIT | 1; // Communication error

      static constexpr uint8_t _SM_HANDSHAKE__S0                = _SM_GROUP_2_HANDSHAKE | 0;
      static constexpr uint8_t _SM_HANDSHAKE__S1                = _SM_GROUP_2_HANDSHAKE | 1;
      static constexpr uint8_t _SM_HANDSHAKE__S2                = _SM_GROUP_2_HANDSHAKE | 2;
      static constexpr uint8_t _SM_HANDSHAKE__S3                = _SM_GROUP_2_HANDSHAKE | 3;

      static constexpr uint8_t _SM_READ__S0                     = _SM_GROUP_3_READ | 0;
      static constexpr uint8_t _SM_READ__S1                     = _SM_GROUP_3_READ | 1;
      static constexpr uint8_t _SM_READ__S2                     = _SM_GROUP_3_READ | 2;

      // this error happens when wrong D2,D3 input (totally unexpected and most likely caused by hardware failure)
      static constexpr uint8_t _SM_RARE__ERROR_INPUT_COMBINATION = _SM_GROUP_5_RARE | 0;
      
      __attribute__((always_inline))
      static void inline _interruptHandler() {
        // clear WDT to prevent it from restarting connection
        __clearSF(_SF_WDT);
        LineState ls = _readLine();
        uint8_t state = _lineTransitionsAsArr[(uint8_t)ls];
        if (state == _SM_NOP) {
          // FIXME For interrupt way of handling things - this will never happen! Remove this state - or make it an error (cause it should not happen)
        } else {
          uint8_t group = state & 0xF0;
          if (group == _SM_GROUP_1_CRIT) {
            _handleCritStates(state);
          } else if (group == _SM_GROUP_2_HANDSHAKE) {
            _handleHandshakeStates(state);
          } else if (group == _SM_GROUP_3_READ) {
            _handleReadStates(state, ls);
          } else if (group == _SM_GROUP_4_WRITE) {
            // FIXME Implement it!
          } else if (group == _SM_GROUP_5_RARE) {
            // FIXME Implement it!
          }
        }
      }

      __attribute__((always_inline))
      static void inline _handleCritStates(uint8_t state) {
        // FIXME Order IFs
        if (state == _SM_CRIT__WAITING_FOR_CONNECTION) {                    // Line is M
          // Tx is connected
          __setSF(_SF_IS_CONNECTED);
          __lineExpectsLMH(_SM_HANDSHAKE__S0, _SM_NOP, _SM_CRIT__COMMERROR);// => waiting for L, H will produce error
          
        } else if (state == _SM_CRIT__COMMERROR) {                          // Line is L,M,H
          __setSF(_SF_IS_ERROR);
          _deactivatePD(); // show Tx that we give-up
          __clearSF(_SF_IS_CONNECTED);
          __lineExpectsLMH(_SM_NOP, _SM_NOP, _SM_NOP);                      // => None of states will do any action till timer expires
          
          // Enable CommError timer, after it elapses - restart communication
          _commErrorTimer = ClockLR::tick();
          __setSF(_SF_COMMERROR_TIMER_ACTIVE);
        }
      }

      __attribute__((always_inline))
      static void inline _handleHandshakeStates(uint8_t state) {
        if (state == _SM_HANDSHAKE__S0) {                                   // Line is L
          __lineExpectsLMH(_SM_NOP, _SM_HANDSHAKE__S1, _SM_CRIT__COMMERROR);// => waiting for M, H will produce error
          
        } else if (state == _SM_HANDSHAKE__S1) {                            // Line is M
          __lineExpectsLMH(_SM_CRIT__COMMERROR, _SM_NOP, _SM_HANDSHAKE__S2);// => waiting for H, L will produce error
          
        } else if (state == _SM_HANDSHAKE__S2) {                            // Line is H
          __lineExpectsLMH(_SM_CRIT__COMMERROR, _SM_HANDSHAKE__S3, _SM_NOP);// => waiting for M, L will produce error
          
        } else if (state == _SM_HANDSHAKE__S3) {                            // Line is M
          // Handshake complete, now prepare everything for Reading!
          _readingDataBuf = 0;
          _readingDataBit = 0;
          __lineExpectsLMH(_SM_READ__S0, _SM_NOP, _SM_READ__S0);            // => waiting for L or H
        }
      }

      __attribute__((always_inline))
      static void inline _handleReadStates(uint8_t state, LineState ls) {
        // FIXME Order IFs
        if (state == _SM_READ__S0) {                                        // Line is L or H
          if (ls == L) {                                                    //    is L
            __lineExpectsLMH(_SM_NOP, _SM_READ__S1, _SM_CRIT__COMMERROR);   // => waiting for M, H will produce error
            
          } else {                                                          //    is H
            _readingDataBuf |= 1 << _readingDataBit;
            __lineExpectsLMH(_SM_CRIT__COMMERROR, _SM_READ__S1, _SM_NOP);   // => waiting for M, L will produce error
          }
          
        } else if (state == _SM_READ__S1) {                                 // Line is M
          _readingDataBit++;
          if (_readingDataBit < 8) {
            // keep reading
            __lineExpectsLMH(_SM_READ__S0, _SM_NOP, _SM_READ__S0);          // => waiting for L or H
          } else {
            _receivedData = _readingDataBuf;
            __setSF(_SF_FRESH_DATA);
            
            // Switch to Writing
            // FIXME Implement this!!!
            //_state = WRITING;
          }
        }
      }

      static void _restartConnection() {
        __clearSF(_SF_IS_CONNECTED);
        __clearSF(_SF_IS_ERROR);
        _activatePD();
        delay(1); // short delay to let line settle // FIXME [CRITICAL] Add delay on TX Side to wait before initiating handshake
        __lineExpectsLMH(_SM_NOP, _SM_CRIT__WAITING_FOR_CONNECTION, _SM_CRIT__COMMERROR);
      }

    public:

      static void setup() {
        _lineTransitions._u = _SM_RARE__ERROR_INPUT_COMBINATION;
        _initializePorts();

        _restartConnection();
        
        attachInterrupt(digitalPinToInterrupt(PORTD_PIN), _interruptHandler, CHANGE);
        attachInterrupt(digitalPinToInterrupt(PORTD_PIN_CMP), _interruptHandler, CHANGE);
      }

      // Assumes ClockLR was updated! // FIXME We need this assertion happen at compile time!
      static void tick() {
        if (__isSFSet(_SF_COMMERROR_TIMER_ACTIVE)
              && ClockLR::isElapsed(_commErrorTimer, ERROR_COMMUNICATION_TIMEOUT_MS)) {
                
          __clearSF(_SF_COMMERROR_TIMER_ACTIVE);
          _restartConnection();
        }

        // WDT
        if (!isConnected()) __clearSF(_SF_WDT);
        if (!__isSFSet(_SF_WDT)) {
          // wdt flag was cleared - we can reset timer
          __setSF(_SF_WDT);
          _wdtTimer = ClockLR::now;
        }
        if (ClockLR::isElapsed(_wdtTimer, COMMUNICATION_STALL_TIMEOUT_MS)) {
          __clearSF(_SF_WDT);
          _restartConnection();
        }
      }

      static bool isConnected() {
        return __isSFSet(_SF_IS_CONNECTED);
      }

      static bool isError() {
        return __isSFSet(_SF_IS_ERROR);
      }

      static bool isFreshData() {
        return __isSFSet(_SF_FRESH_DATA);
      }

      static uint8_t getData() {
        uint8_t data;
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
          __clearSF(_SF_FRESH_DATA);
          data = _receivedData;
        }
        return data;
      }
      
  };

  template <uint8_t P, uint8_t C, uint8_t PD>
  volatile uint8_t OneWireErrorReceiver<P,C,PD>::_statusFlags = 0;
  template <uint8_t P, uint8_t C, uint8_t PD>
  volatile uint8_t OneWireErrorReceiver<P,C,PD>::_receivedData = 0;
  template <uint8_t P, uint8_t C, uint8_t PD>
  uint8_t OneWireErrorReceiver<P,C,PD>::_readingDataBuf = 0;
  template <uint8_t P, uint8_t C, uint8_t PD>
  uint8_t OneWireErrorReceiver<P,C,PD>::_readingDataBit = 0;
  template <uint8_t P, uint8_t C, uint8_t PD>
  typename OneWireErrorReceiver<P,C,PD>::LineThenState OneWireErrorReceiver<P,C,PD>::_lineTransitions;
  template <uint8_t P, uint8_t C, uint8_t PD>
  uint16_t OneWireErrorReceiver<P,C,PD>::_commErrorTimer = 0;
  template <uint8_t P, uint8_t C, uint8_t PD>
  uint16_t OneWireErrorReceiver<P,C,PD>::_wdtTimer = 0;
  template <uint8_t P, uint8_t C, uint8_t PD>
  uint8_t* const OneWireErrorReceiver<P,C,PD>::_lineTransitionsAsArr = 
                    (uint8_t*)&OneWireErrorReceiver<P,C,PD>::_lineTransitions;

}

using ErrorReceiver = pinkyvolt::debug::rx::OneWireErrorReceiver<2, 3, 4>;

#endif
