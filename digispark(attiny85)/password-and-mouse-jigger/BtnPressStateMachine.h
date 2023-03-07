// FIXME This must be a part of my library!!!
// This is a copy of same file in "arduino-sketch-war-comm-system-2"
#ifndef BtnPressStateMachine_h
#define BtnPressStateMachine_h

#include "Arduino.h"
#include "TimerChecker.h"

class BtnPressStateMachine {
  private:
    static uint8_t buttonIdGen;

    uint8_t buttonId;// Id of button (for debugging purposes)
    uint8_t pinBtn;
    bool defaultBtnState = true;

    typedef enum {
      ZERO = 0,
      PRESSED,
      RELEASED, // FIXME We need to delay after RELEASED (to switch to real RELEASED, because it might be contact de-bouncing)
      COOLDOWN // during this time, event should be taken (processed)
    } btn_press_state;

    uint8_t state = ZERO;
    TimerChecker timerChecker;
    uint16_t v = 0;
    uint16_t result = 0;

    void _reset() {
      state = ZERO;
      v = 0;
      result = 0;
    };
    const static uint8_t intervalMs = 128;

  public:
    BtnPressStateMachine(uint8_t pinBtn, bool defaultBtnState): timerChecker(intervalMs) {
      buttonId = buttonIdGen++;
      this->pinBtn = pinBtn;
      this->defaultBtnState = defaultBtnState;
    }
    void loop();

    inline bool isPressed() __attribute__((always_inline)) { return state == PRESSED; }
    void setResult(uint16_t res) { result = res; } // FIXME Must be private access!!!
    inline bool isResult() __attribute__((always_inline)) { return result != 0; }
    uint16_t takeResult() { uint16_t r = result; result = 0; return r; }
    uint16_t takeResultMs() { return takeResult() << 7; }
    inline uint16_t peekResult() __attribute__((always_inline)) { return v; }
    inline uint16_t peekResultMs() __attribute__((always_inline)) { return peekResult() << 7; }
    void reset() { _reset(); }
};

#endif