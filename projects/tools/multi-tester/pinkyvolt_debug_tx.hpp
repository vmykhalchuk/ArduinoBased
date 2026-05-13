#ifndef PINKYVOLT_DEBUG_TX_H
#define PINKYVOLT_DEBUG_TX_H

#include <Arduino.h>
#include <util/atomic.h>
#include "clock.h"

namespace pinkyvolt::debug::tx {
  

                             //   A A P _
                             //  _H_L_U_Z
  constexpr uint8_t __Z_to__ = 0B00110100;
  constexpr uint8_t _PU_to__ = 0B11000001;
  constexpr uint8_t _AL_to__ = 0B01000010;
  constexpr uint8_t _AH_to__ = 0B00011000;
                      //=> 2bit opCodes:
                      //        00->not allowed(error)    01->toggle PIN
                      //        10->set as input          11->set as output
  constexpr uint8_t _TRANSITION_MAP[] = {__Z_to__, _PU_to__, _AL_to__, _AH_to__};

  template <uint8_t PORTD_PIN>
  class OneWireErrorTransmitter final {
    
    private:

      static constexpr bool __debug = true;
    
      static_assert((PORTD_PIN <= 7), "PORTD_PIN must be within 0..7 range!");

      static constexpr uint8_t _MASK = 1<<PORTD_PIN;

      enum FMStateGroup {
        _G_CRIT = 0x10,
        _G_HS   = 0x20,
        _G_WR   = 0x30,
        _G_RD   = 0x40
      };
      
      enum FSMState {
        // Assures Receiver has min 4uS time to react. Note: It can be way above that time - all depends on how often tick() is called
        _SPEC__SKIP_FULL_CYCLE              = 0 | 0,
        
      //_CRIT__SKIP_FULL_CYCLE              = _G_CRIT | 0, // Moved to Special case
        _CRIT__WAITING_FOR_HANDSHAKE        = _G_CRIT | 0,
        _CRIT__COMMERROR                    = _G_CRIT | 1,
        _CRIT__SKIP_CURRENT_CYCLE           = _G_CRIT | 2, // TODO Redundant
        _CRIT__SKIP_TWO_FULL_CYCLES         = _G_CRIT | 3, // TODO Redundant
        _CRIT__LONG_WAIT                    = _G_CRIT | 4,
        _CRIT__ALG_ERROR_TXOUT              = _G_CRIT | 0xC, // Algorithimc error - TX Out
        _CRIT__ALG_ERROR__01                = _G_CRIT | 0xD, // Algorithimc error - unhandled logic state

        // HANDSHAKE
        _HS__S1                             = _G_HS | 1,
        _HS__S2                             = _G_HS | 2,
        _HS__S3                             = _G_HS | 3,
        _HS__S4                             = _G_HS | 4,
        _HS__S5                             = _G_HS | 5,

        // WRITING
        _WR_START                           = _G_WR | 0,
        _WR_S0                              = _G_WR | 1,
        _WR_S1                              = _G_WR | 2,
        _WR_S2                              = _G_WR | 3,
        _WR_S3                              = _G_WR | 4,
      //_WR_ERROR                           = _G_WR | 5,

        // READING
        _RD_START                           = _G_RD | 0,
        _RD_S0                              = _G_RD | 1,
        _RD_S1                              = _G_RD | 2,
        _RD_S2                              = _G_RD | 3,
        _RD_S3_L                            = _G_RD | 4,
        _RD_S4_H                            = _G_RD | 5,
        _RD_S5                              = _G_RD | 6,
        _RD_ERROR                           = _G_RD | 7,

        _NOOP                               = 0xFF
      };

      FSMState _whenL_then = _NOOP;
      FSMState _whenH_then = _NOOP;
      
      FSMState _whenL_then__saved4waitFullCycle = _NOOP;
      
      void _waitFullCycleAndSwitchToLH(FSMState whenL_then, FSMState whenH_then) {
        _whenL_then = _SPEC__SKIP_FULL_CYCLE;
        _whenL_then__saved4waitFullCycle = whenL_then;
        _whenH_then = whenH_then;
        
        asm volatile("" ::: "memory");
        _skipCycleTmr0p1 = _skipCycleTmr0 = TCNT0;
        _skipCycleTmr0p1++;
      }
      
      void _switchToLH(FSMState whenL_then, FSMState whenH_then) {
        _whenL_then = whenL_then;
        _whenH_then = whenH_then;
      }

      // Set D<PORTD_PIN> To Output (1 cycle)
      inline static void setDPinToOutput() { asm volatile ("sbi %0, %1" : : "I" (_SFR_IO_ADDR(DDRD)), "I" (PORTD_PIN) : "memory"); }

      // Set D<PORTD_PIN> To Output (1 cycle)
      inline static void setDPinToInput() { asm volatile ("cbi %0, %1" : : "I" (_SFR_IO_ADDR(DDRD)), "I" (PORTD_PIN) : "memory"); }

      // Set D<PORTD_PIN> HIGH (1 cycle)
      inline static void setDPinToHigh() { asm volatile ("sbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTD)), "I" (PORTD_PIN) : "memory"); }

      // Set D<PORTD_PIN> LOW (1 cycle)
      inline static void setDPinToLow() { asm volatile ("cbi %0, %1" : : "I" (_SFR_IO_ADDR(PORTD)), "I" (PORTD_PIN) : "memory"); }

      // Toggle D<PORTD_PIN> (1 cycle)
      // Writing a 1 to the PIN register toggles the PORT bit
      inline static void toggleDPinState() { asm volatile ("sbi %0, %1" : : "I" (_SFR_IO_ADDR(PIND)), "I" (PORTD_PIN) : "memory"); }

      inline static bool isDPinHigh() { return PIND & _MASK; }

      inline static bool isDPinLow() { return !(PIND & _MASK); }

      enum TXMode {
        __Z, //00
        _PU, //01
        _AL, //10
        _AH, //11
        TXModeCount
      };
      
      /*TXMode _currTXMode = __Z;

      __attribute__((always_inline))
      inline void transitionTX(TXMode changeTo) {
        static_assert(TXModeCount==4, "TXModeCount must be = 4!");

        uint8_t _trnst = _TRANSITION_MAP[(uint8_t)_currTXMode];
        uint8_t opCode = _trnst >> ((uint8_t)changeTo<<1);
        opCode &= 0x3;

        switch (opCode) {
          case 0: // not allowed (error)
            _switchToLH(_CRIT__ALG_ERROR_TXOUT,_CRIT__ALG_ERROR_TXOUT);
            break;
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

        _currTXMode = changeTo;
      }*/

      inline static TXMode getTx() {
        uint8_t dir = ((DDRD >> PORTD_PIN) & 1) << 1;
        uint8_t state = ((PORTD >> PORTD_PIN) & 1);
        return (TXMode)(dir|state);
      }

      inline static void tx_Z2PU() {
        if (__debug && (getTx() != __Z)) {
          // FIXME Add some debugging here - maybe print to Serial
        }
        toggleDPinState();
      }
      inline static void tx_Z2AL() {
        if (__debug && (getTx() != __Z)) {
          // FIXME Add some debugging here - maybe print to Serial
          return;
        }
        setDPinToOutput();
      }
      //inline static void tx_Z2AH() IMPOSSIBLE With single transition
      
      inline static void tx_PU2Z() {
        if (__debug && (getTx() != _PU)) {
          // FIXME Add some debugging here - maybe print to Serial
          return;
        }
        toggleDPinState();
      }
      inline static void tx_PU2AH() {
        if (__debug && (getTx() != _PU)) {
          // FIXME Add some debugging here - maybe print to Serial
          return;
        }
        setDPinToOutput();
      }
      //inline static void tx_PU2AL() IMPOSSIBLE With single transition

      inline static void tx_AL2Z() {
        if (__debug && (getTx() != _AL)) {
          // FIXME Add some debugging here - maybe print to Serial
          return;
        }
        setDPinToInput();
      }
      inline static void tx_AL2AH() {
        if (__debug && (getTx() != _AL)) {
          // FIXME Add some debugging here - maybe print to Serial
          return;
        }
        toggleDPinState();
      }
      //inline static void tx_AL2PU() IMPOSSIBLE With single transition

      inline static void tx_AH2PU() {
        if (__debug && (getTx() != _AH)) {
          // FIXME Add some debugging here - maybe print to Serial
          return;
        }
        setDPinToInput();
      }
      inline static void tx_AH2AL() {
        if (__debug && (getTx() != _AH)) {
          // FIXME Add some debugging here - maybe print to Serial
          return;
        }
        toggleDPinState();
      }
      //inline static void tx_AH2Z() IMPOSSIBLE With single transition


      //SMState _skipCycleReturnState = ERROR;
      uint8_t _skipCycleTmr0 = 0;
      uint8_t _skipCycleTmr0p1 = 0;
      uint8_t _skipCycleCounter = 0;

/*
          // FIXME Clean-up below (Preserve skipTwoFullCycles)

      // Assures Receiver has min 8uS time to react (1 TCNT0 increment is 4uS)
      inline void skipTwoFullCycles(SMState retState) {
        _skipCycleReturnState = retState;
        _skipCycleCounter = 0;
        _state = SKIP_TWO_FULL_CYCLES;

        asm volatile("" ::: "memory");
        _skipCycleTmr0 = TCNT0;
      }

          // Assures Receiver has min 8uS time to react (1 TCNT0 increment is 4uS)
          case SKIP_TWO_FULL_CYCLES:
            if (TCNT0 - _skipCycleTmr0 > 2) {
              _state = _skipCycleReturnState; // done waiting
            } else {
              if (++_skipCycleCounter > 20) { // 20 * 10 instructions (less is physically impossible) = 200*62.5nS=12.5uS
                // This is fall-back mechanism
                // With 3 / 256 timer states - the chances are that some rhytmic calls will leave SM in this state for very long
                _state = _skipCycleReturnState; // done waiting
              }
            }
            break;
  
*/

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
        //_currTXMode = _PU;
        }
        _waitFullCycleAndSwitchToLH(_CRIT__WAITING_FOR_HANDSHAKE, _CRIT__WAITING_FOR_HANDSHAKE);
      }


      void tick() {
        if (_whenL_then == _SPEC__SKIP_FULL_CYCLE) {
          // we first check for +1 cycle, we expect in most cases user sketch will be slow
          if (_skipCycleTmr0p1 != TCNT0) {
            if (_skipCycleTmr0 != TCNT0) {
              _whenL_then = _whenL_then__saved4waitFullCycle;
            } else {
              // we are still at the beginning - wait more (apparently this is test sketch, testing for speed performance)
            }
          }
        } else {
          bool isLow = isDPinLow();
          if (_whenL_then == _CRIT__WAITING_FOR_HANDSHAKE) {
            if (isLow) _waitFullCycleAndSwitchToLH(_HS__S1, _CRIT__WAITING_FOR_HANDSHAKE);
          } else {
            _lineStateChangeHandler(isLow);
          }
        }
      }

    private:

      __attribute__((always_inline))
      inline void _lineStateChangeHandler(bool isLow) {
        FSMState state = isLow ? _whenL_then : _whenH_then;
        uint8_t group = state & 0xF0;

        if (_G_CRIT == group) {
          _handleCritState(state);
        } else if (_G_HS == group) {
          _handleHandshakeState(state);
        } else if (_G_WR == group) {
          _handleWritingState(state);
        } else if (_G_RD == group) {
          _handleReadingState(state);

        } else {
           _switchToLH(_CRIT__ALG_ERROR__01, _CRIT__ALG_ERROR__01);
        }
      }

      __attribute__((always_inline))
      inline void _handleCritState(FSMState state) {
        if (_CRIT__WAITING_FOR_HANDSHAKE == state) { // line == M (PD+PU) || line == H (Z+PU)
          //_switchToLH(_HS__S1, _CRIT__WAITING_FOR_HANDSHAKE); // When L(M)(PD+PU) - handshake begins
          _switchToLH(_CRIT__COMMERROR, _CRIT__COMMERROR); // Is handled in tick() method since this is most often active state, here it comes only when some error happens
          
        } else if (_CRIT__COMMERROR == state) {// Line == <ANY>
          // FIXME - now we need to convert line to Z, and when time out finishes - to PU!
          // FIXME Handle Communication Error here! (Set time-out or whatever)

        } else if (_CRIT__ALG_ERROR_TXOUT == state) {// Line == <ANY>
          // FIXME Handle

        } else if (_CRIT__ALG_ERROR__01 == state) {// Line == <ANY>
          // FIXME Handle

        } else {
           _switchToLH(_CRIT__ALG_ERROR__01, _CRIT__ALG_ERROR__01);
        }
      }

      __attribute__((always_inline))
      inline void _handleHandshakeState(FSMState state) {
        
        if (_HS__S1 == state) {               // line == M (PD+PU)
          // FIXME return expected new TX state - and change in single location!
          tx_PU2Z();                          // TX=>Z; Line=L (PD+ Z)
          _waitFullCycleAndSwitchToLH(_HS__S2, _CRIT__COMMERROR);
          
        } else if (_HS__S2 == state) {        // line == L (PD+ Z)
          tx_Z2PU();                          // TX=>PU; Line=M (PD+PU)
          _waitFullCycleAndSwitchToLH(_HS__S3, _CRIT__COMMERROR);

        } else if (_HS__S3 == state) {        // line == M (PD+PU)
          tx_PU2AH();                         // TX=>AH; Line=H (PD+AH)
          // After this moment Receiver can verify if its Blind or not to M state of line. If Receiver can see HIGH unexpectedly - it will switch to give-up state
          _waitFullCycleAndSwitchToLH(_CRIT__COMMERROR, _HS__S4);
          
        } else if (_HS__S4 == state) {        // line == H (PD+AH)
          tx_AH2PU();                         // TX=>PU; Line=M (PD+PU) (OR Line=H (Z+PU) if Receiver noticed unexpected High, and deducted that it missed M/L/M transitions)
          _waitFullCycleAndSwitchToLH(_HS__S5, _CRIT__COMMERROR);
          
        } else if (_HS__S5 == state) {        // line == M (PD+PU)
          _switchToLH(_WR_START, _CRIT__COMMERROR);

        } else {
           _switchToLH(_CRIT__ALG_ERROR__01, _CRIT__ALG_ERROR__01);
        }
      }

      __attribute__((always_inline))
      inline void _handleWritingState(FSMState state) {

        if (_WR_START == state) {             // Line == M (PD+PU)
          ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            _writingErrorFlags = _errorFlags;
            _errorFlags = 0; // reset - if writing fails we revert _errorFlags with _writeErrorFlags <= FIXME Implement that logic
          }
          _writingFlag = 0; // start from bit 0
          _switchToLH(_WR_S0, _RD_ERROR);
          
        } else if (_WR_S0 == state) {         // Line == M (PD+PU)
          if (_writingErrorFlags & (1 << _writingFlag)) {
            // Write 0
            tx_PU2Z();                        // TX=>Z; Line=L (PD+Z)
            _waitFullCycleAndSwitchToLH(_WR_S1, _RD_ERROR);
          } else {
            // Write 1
            tx_PU2AH();                       // TX=>AH; Line=H (PD+AH)
            _waitFullCycleAndSwitchToLH(_RD_ERROR, _WR_S2);
          }
          
        } else if (_WR_S1 == state) {         // Line == L (PD+Z)
          tx_Z2PU();                          // TX=>PU; Line=M (PD+PU)
          _waitFullCycleAndSwitchToLH(_WR_S3, _RD_ERROR);
          
        } else if (_WR_S2 == state) {         // Line == H (PD+AH)
          tx_AH2PU();                         // TX=>PU; Line=M (PD+PU)
          _waitFullCycleAndSwitchToLH(_WR_S3, _RD_ERROR);
          
        } else if (_WR_S3 == state) {         // Line == M (PD+PU)
          _writingFlag++;
          if (_writingFlag <= 7) {
            _switchToLH(_WR_S0, _RD_ERROR);
          } else {
            // last bit was written - exit
            _switchToLH(_RD_START, _RD_ERROR);
          }
          
        //} else if (_WR_ERROR == state) {       // Line == <ANY>
        //  registerErrorsInBulk(_writingErrorFlags); // FIXME - we should do this after Reading back!
        //  _switchToLH(_CRIT__COMMERROR, _CRIT__COMMERROR);

        } else {
           _switchToLH(_CRIT__ALG_ERROR__01, _CRIT__ALG_ERROR__01);
        }
      }
      
      __attribute__((always_inline))
      inline void _handleReadingState(FSMState state) {

        if (_RD_START == state) {              // Line == M (PD+PU)
          _readingData = 0;
          _readingBit = 0;
          _switchToLH(_RD_S0, _RD_ERROR);
          
        } else if (_RD_S0 == state) {          // Line == M (PD+PU)
          tx_PU2AH();                          // TX=>AH; Line=H (PD+AH)
          _waitFullCycleAndSwitchToLH(_RD_ERROR, _RD_S1);

        } else if (_RD_S1 == state) {          // Line == H (PD+AH)
          tx_AH2AL();                          // TX=>AL; Line=L (PD+AL) (No possible action on Receiver side can change Line state, so it is safe to transition here in two steps)
          tx_AL2Z();                           // TX=> Z; Line=L (PD+Z)
          _waitFullCycleAndSwitchToLH(_RD_S2, _RD_ERROR);

        } else if (_RD_S2 == state) {          // Line == L (PD+Z)
          tx_Z2PU();                           // TX=>PU; Line=M (PD+PU)
          _waitFullCycleAndSwitchToLH(_RD_S3_L, _RD_S4_H);
          
        } else if (_RD_S3_L == state) {        // Line == M (PD+PU) // Receiver decided on 0
          tx_PU2Z();                           // TX=> Z; Line=L (PD+Z) // FIXME Make Sure Receiver DOESN'T ACT AT THIS M=>L TRANSITION!!! (Receiver must wait till line becomes M again)
          tx_Z2AL();                           // TX=>AL; Line=L (PD+AL)
          _waitFullCycleAndSwitchToLH(_RD_S5, _RD_ERROR);

        } else if (_RD_S4_H == state) {        // Line == H (Z+PU) // Receiver decided on 1
          _readingData |= 1 << _readingBit;
          tx_PU2AH();                          // TX=>AH; Line=H (Z+AH)
          tx_AH2AL();                          // TX=>AL; Line=L (Z+AL)
          _waitFullCycleAndSwitchToLH(_RD_S5, _RD_ERROR);
          
        } else if (_RD_S5 == state) {          // Line == L (PD+AL)
          tx_AL2Z();                           // TX=>Z; Line=L (PD+Z)
          tx_Z2PU();                           // TX=>PU; Line=M (PD+PU)
          if (_readingBit < 3) { // we read 4 bits only (configure same for Receiver!)
            _readingBit++;
            _waitFullCycleAndSwitchToLH(_RD_S0, _RD_ERROR);
          } else {
            // last bit was read - exit
            // FIXME Validate readingData with _writingErrorFlags! If validation fails -> go to _RD_ERROR
            _waitFullCycleAndSwitchToLH(_CRIT__WAITING_FOR_HANDSHAKE, _CRIT__WAITING_FOR_HANDSHAKE); // FIXME Make sure we can restart as usual (TX Port should come back to PU)
          }
          
        } else if (_RD_ERROR == state) {       // Line == <ANY>
          registerErrorsInBulk(_writingErrorFlags);
          _switchToLH(_CRIT__COMMERROR, _CRIT__COMMERROR);

        } else {
           _switchToLH(_CRIT__ALG_ERROR__01, _CRIT__ALG_ERROR__01);
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

#endif
