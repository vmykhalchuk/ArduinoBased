#ifndef PINKYVOLT_DEBUG_RX_H
#define PINKYVOLT_DEBUG_RX_H

#include <Arduino.h>
#include <util/atomic.h>
#include "clock.h"

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
