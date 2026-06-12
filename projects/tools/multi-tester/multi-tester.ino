/*

  Multi-tester with set of programs to operate accordingly (see _readme.h)

  Libraries required:
    - PinkyvoltToolkit v0.1.0
    - LongLiveThEEPROM v0.1.1
 */

#include <Arduino.h>
#include <pref_one_byte.h>
#include <clock.h>
#include <ds18b20.h>
#include <input_button.h>
#include <pvt_adc_precise.hpp>
#include <pvt_debug.hpp>
#include "htu21d.h" // Uses A4,A5
#include "kh2441ef.h" // Pins used 5,6,7,8,9,10

using ErrorRx = pvt::ErrorReceiver<2,3>; // Pins used 2 (Line Read) and 3 (Strong Up/Down Pull)

InputButton::Def btnSelect = { .pinNo = 11, .isActiveHigh = false, .enablePullup = true, ._ctx = {}};
InputButton::Def btnExit = { .pinNo = 12, .isActiveHigh = false, .enablePullup = true, ._ctx = {}};

constexpr int tempSensorDS18B20Pin = 4; // FIXME Move to other pin!

constexpr int ADC_MONITOR_PIN = A0;

PrefOneByteParanoia prefStore;

void freezeAndDisplayEEPROMError() {
  uint8_t prefStoreError = (uint8_t) prefStore.getLastError();
  
  uint16_t timerMs = ClockLR::tick();
  uint8_t displMsg = 0; // EPr -> Err -> <Code>
  uint8_t sel = KH2441EF::S_SEL1;
  while (true) { // FIXME Make it possible to reset EEPROM from here! For example by pressing two buttons for T > LONG_PRESS_DURATION_MS
    KH2441EF::tick();
    ClockLR::tick();
    InputButton::tick(btnSelect);
    if (ClockLR::isElapsed(timerMs, 700)) {
      switch (displMsg) {
        case 0: KH2441EF::setDisplayBuf(sel, KH2441EF::S_E, KH2441EF::S_P, KH2441EF::S_r, false); break;
        case 1: KH2441EF::setDisplayBuf(sel, KH2441EF::S_E, KH2441EF::S_r, KH2441EF::S_r, false); break;
        case 2: KH2441EF::setDisplayBuf(sel, KH2441EF::S_UND, prefStoreError/10%10, prefStoreError%10, false); break;
        default:
          displMsg = 0;
      }
      displMsg = displMsg == 2 ? 0 : displMsg + 1;
      sel = sel == KH2441EF::S_SEL1 ? KH2441EF::S_SEL2 : KH2441EF::S_SEL1;
      timerMs = millis();
    }
  }
}
  
void setup() {
  
  uint8_t progNo = prefStore.load();
  if (!prefStore.isSuccess()) {
    freezeAndDisplayEEPROMError();
  }
  if (prefStore.isEmpty()) progNo = 0;
  
  uint16_t setupTimerMs = ClockLR::tick();
  while (!ClockLR::isElapsed(setupTimerMs, InputButton::LONG_PRESS_DURATION_MS << 1)) {
    ClockLR::tick();
    InputButton::tick(btnSelect);
    InputButton::tick(btnExit);
    KH2441EF::tick();
    if (InputButton::isLongPressed(btnSelect)) {
      // enter select program mode
      progNo = selectProgramMode(progNo, false);
      break;
    }
  };

  if (progNo == 99) {
    // Now we will endlessly select program and save it to EEPROM
    // Press RESET to exit this loop!
    while (true) {
    
      progNo = selectProgramMode(progNo, true);
      prefStore.save(progNo);
      if (!prefStore.isSuccess()) {
        freezeAndDisplayEEPROMError();
      }
      
      setupTimerMs = ClockLR::tick();
      uint8_t i = 0;
      while (true) {
        ClockLR::tick();
        InputButton::tick(btnSelect);
        InputButton::tick(btnExit);
        KH2441EF::tick();
        if (ClockLR::isElapsed(setupTimerMs, 300)) {
          //updateDisplayWithProgNo(progNo, i % 2, i % 2, true);
          KH2441EF::setDisplayBuf(i % 2 ? KH2441EF::S_SEL2 : KH2441EF::S_BLANK,
                        i % 2 ? KH2441EF::S_BLANK : KH2441EF::S_S,
                        i % 2 ? KH2441EF::S_BLANK : KH2441EF::S_u,
                        i % 2 ? KH2441EF::S_BLANK : KH2441EF::S_d,
                        false);
          setupTimerMs = ClockLR::tick();
          i++;
          if (i > 40) break;
        }
      }
    }
  }
  
  // Display progNo for 1.5s before starting it
  updateDisplayWithProgNo(progNo, true, false);
  setupTimerMs = ClockLR::tick();
  while (!ClockLR::isElapsed(setupTimerMs, 1500)) {
    ClockLR::tick();
    KH2441EF::tick();
  }
  KH2441EF::muteDisplayInstantly();
  
  while(true) loop(progNo);
}

void loop() {}

enum SelProgState { NOT_INTLZD, IDLE, WAITING_4DOUBLE_CLICK, WAITING_4EXIT };
uint8_t selectProgramMode(uint8_t startWithProgNo, bool forSave99) {
  uint8_t pn = startWithProgNo;
  bool isOn = false;
  SelProgState state = NOT_INTLZD;
  KH2441EF::setDisplayBuf(KH2441EF::S_MINUS, KH2441EF::S_MINUS, KH2441EF::S_MINUS, KH2441EF::S_MINUS, false);
  uint16_t doubleClickStartMs, displayBlinkStartMs = ClockLR::tick();
  while (true) {
    ClockLR::tick();
    InputButton::tick(btnSelect);
    InputButton::tick(btnExit);
    KH2441EF::tick();
    if (ClockLR::isElapsed(displayBlinkStartMs, 700)) {
      displayBlinkStartMs = ClockLR::now;
      // update screen
      isOn = true;//!isOn; // lets stop this blinking
      updateDisplayWithProgNo(pn, isOn, forSave99);
    }
    bool _exit = false;
    switch (state) {
      case NOT_INTLZD:
        if (!InputButton::isLongPressed(btnSelect)) {
          state = IDLE;
          InputButton::wasPressed(btnSelect); // erase wasPressed flag (stays on after longPressed)
          InputButton::wasReleased(btnSelect); // this is hack
        }
      break;
      case IDLE:
        if (InputButton::wasReleased(btnSelect)) {
          doubleClickStartMs = ClockLR::now;
          state = WAITING_4DOUBLE_CLICK;
        }
        if (InputButton::isLongPressed(btnSelect)) {
          state = WAITING_4EXIT;
        }
      break;
      case WAITING_4DOUBLE_CLICK:
        if (InputButton::wasReleased(btnSelect)) {
          // DoubleClick
          pn = incrementProgNo(pn, true);
          updateDisplayWithProgNo(pn, isOn, forSave99);
          state = IDLE;
        }
        if (ClockLR::isElapsed(doubleClickStartMs, 600)) {
          // No DoubleClick
          pn = incrementProgNo(pn, false);
          updateDisplayWithProgNo(pn, isOn, forSave99);
          state = IDLE;
        }
      break;
      case WAITING_4EXIT:
        if (!isOn) isOn=true; // force screen to light constantly
        updateDisplayWithProgNo(pn, isOn, true, forSave99);
        if (!InputButton::isPressed(btnSelect)) {
          InputButton::reset(btnSelect);
          _exit = true;
        }
      break;
    }
    if (_exit) break;
  }
  return pn;
}

void updateDisplayWithProgNo(uint8_t pn, bool isOn, bool forSave99) {
  updateDisplayWithProgNo(pn, isOn, false, forSave99);
}

void updateDisplayWithProgNo(uint8_t pn, bool isOn, bool selectFlag, bool forSave99) {
  KH2441EF::setDisplayBuf(selectFlag || forSave99 ? KH2441EF::S_SEL2 : KH2441EF::S_BLANK,
                          forSave99 ? KH2441EF::S_S : KH2441EF::S_P,
                          isOn ? pn/10 : KH2441EF::S_BLANK,
                          isOn ? pn%10 : KH2441EF::S_BLANK,
                          false);
}

uint8_t incrementProgNo(uint8_t pn, bool doubleClick) {
  uint8_t maj = pn / 10, minor = pn % 10;
  if (doubleClick) {
    minor = 0;
    switch (maj) {
      case 0: maj = 1; break;
      case 1: maj = 2; break;
      case 2: maj = 9; minor = 9; break;
      case 9: maj = 0; break;
      default: maj = 0; minor = 0;
    }
  } else {
    switch (maj) {
      case 0: minor = minor >= 4 ? 0 : minor+1; break;
      case 1: minor = minor >= 3 ? 0 : minor+1; break;
      case 2: minor = minor >= 4 ? 0 : minor+1; break;
      case 9: minor = minor >= 9 ? 9 : minor+1; break;
      default: maj = 0; minor = 0;
    }
  }
  return maj * 10 + minor;
}

uint8_t progsData[] = {0,0};
bool _firstRun = true;
uint16_t progTimerMs;
void loop(uint8_t progNo) {
  if (_firstRun) progTimerMs = ClockLR::tick();
  ClockLR::tick();
  InputButton::tick(btnSelect);
  InputButton::tick(btnExit);
  KH2441EF::tick();
  
  switch(progNo) {
    case 00: tickProg00(progsData[0], progsData[1]); break;
    case 01: tickProg01(progsData[0], progsData[1]); break;
    case 03: tickProg03(progsData[0], progsData[1]); break;
    case 20: tickProg20(progsData[0], progsData[1]); break;
    default:
      KH2441EF::setDisplayBufToErrorMsg();
  }

  if (_firstRun) _firstRun = false;
}

void tickProg00(uint8_t &wasBtnPressed, uint8_t &unused) {
  unused = 0;
  
  bool forceDataRead = false;
  if (_firstRun || InputButton::wasPressed(btnSelect)) {
    wasBtnPressed = true;
    progTimerMs = ClockLR::now;
    KH2441EF::setDisplayBuf(KH2441EF::S_BLANK, KH2441EF::S_t, KH2441EF::S_UND, KH2441EF::S_C, false);
  }
  if (wasBtnPressed && ClockLR::isElapsed(progTimerMs, 600)) {
    wasBtnPressed = false;
    forceDataRead = true;
  }
  if (forceDataRead || ClockLR::isElapsed(progTimerMs, 10000)) {
    KH2441EF::muteDisplayInstantly(); // to prevent some segments to light bright while Temp measurements take place
    float t = DS18B20::readTemperature(tempSensorDS18B20Pin);
    KH2441EF::setDisplayBufToFloatWithOneDecimal(t);
    progTimerMs = ClockLR::tick();
  }
}

void tickProg01(uint8_t &wasBtnPressed, uint8_t &pd0Mode) {
  if (_firstRun) {
    HTU21D::setup();
  }

  bool forceDataRead = false;
  if (_firstRun || InputButton::wasPressed(btnSelect)) {
    wasBtnPressed = true;
    pd0Mode = pd0Mode == 0 ? 1 : 0;
    progTimerMs = ClockLR::now;
    if (pd0Mode == 0) {
      KH2441EF::setDisplayBuf(KH2441EF::S_BLANK, KH2441EF::S_t, KH2441EF::S_UND, KH2441EF::S_C, false);
    } else {
      KH2441EF::setDisplayBuf(KH2441EF::S_BLANK, KH2441EF::S_BLANK, KH2441EF::S_r, KH2441EF::S_h, false);
    }
  }
  if (wasBtnPressed && ClockLR::isElapsed(progTimerMs, 600)) {
    wasBtnPressed = false;
    forceDataRead = true;
  }
  if (forceDataRead || ClockLR::isElapsed(progTimerMs, 10000)) {
    KH2441EF::muteDisplayInstantly(); // to prevent some segments to light bright while Temp measurements take place
    float d;
    if (pd0Mode == 0) {
      d = HTU21D::readTemp();
    } else {
      d = HTU21D::readHum();
    }
    HTU21D::Error err = HTU21D::getError();
    if (err != HTU21D::OK) {
      if (err == HTU21D::NOT_ENOUGH_DATA_RECEIVED) {
        KH2441EF::setDisplayBuf(KH2441EF::S_BLANK, KH2441EF::S_E, KH2441EF::S_r, 0, false);
      } else if (err == HTU21D::CRC_ERROR) {
        KH2441EF::setDisplayBuf(KH2441EF::S_BLANK, KH2441EF::S_c, KH2441EF::S_r, KH2441EF::S_c, false);
      } else {
        KH2441EF::setDisplayBuf(KH2441EF::S_BLANK, KH2441EF::S_E, KH2441EF::S_r, KH2441EF::S_r, false);
      }
    } else {
      KH2441EF::setDisplayBufToFloatWithOneDecimal(d);
    }
    progTimerMs = ClockLR::tick();
  }
}

void tickProg03(uint8_t &unused, uint8_t &unused2) {
  if (_firstRun) {
    unused = 0; unused2 = 0;
  }
  
  if (_firstRun || ClockLR::isElapsed(progTimerMs, 1000)) {
    KH2441EF::muteDisplayInstantly(); // to prevent some segments to light bright while ADC conversion happens
    uint16_t v = pvt::ADCPrecise::preciseAnalogRead(ADC_MONITOR_PIN);
    KH2441EF::setDisplayBufToInt(v);
    progTimerMs = ClockLR::tick();
  }
}

void tickProg20(uint8_t &flags, uint8_t &frameErrorCode) {
  constexpr uint8_t F_READ_NEXT_FRAME = 0x01;
  constexpr uint8_t F_FRAME_RECEIVED  = 0x02;
  constexpr uint8_t F_FRAME_ERROR     = 0x04;

  if (_firstRun) {
    flags |= F_READ_NEXT_FRAME;
    ErrorRx::setup();
  }
  
  bool updateDisplay = false;
  if (flags & F_READ_NEXT_FRAME) {
    flags &= ~F_READ_NEXT_FRAME;
    
    KH2441EF::muteDisplayInstantly();
    KH2441EF::setDisplayBuf(KH2441EF::S_BLANK, KH2441EF::S_BLANK, KH2441EF::S_BLANK, KH2441EF::S_BLANK, true);
    uint8_t err = 0, subErr = 0;
    bool success = ErrorRx::readFrame(0x10, err, subErr); // block for long time
    flags &= ~F_FRAME_RECEIVED;
    if (success) {
      flags |= F_FRAME_RECEIVED;
    }
    flags &= ~F_FRAME_ERROR;
    if (err != 0) {
      flags |= F_FRAME_ERROR;
      frameErrorCode = err;
    }
    
    InputButton::reset(btnSelect);
    InputButton::reset(btnExit);
    progTimerMs = ClockLR::tick();
    updateDisplay = true;
  }
  
  if (InputButton::isPressed(btnSelect)) {
    flags |= F_READ_NEXT_FRAME;
  }
  
  if (ClockLR::isElapsed(progTimerMs, 1000)) {
    updateDisplay = true;
  }
  
  if (updateDisplay) {
    progTimerMs = ClockLR::tick();
    if (flags & F_FRAME_RECEIVED) {
      bool error = flags & F_FRAME_ERROR;
      if (error) {
        KH2441EF::setDisplayBufToIntAsHex(frameErrorCode, false);
        KH2441EF::updateDisplayBufDigit(2, KH2441EF::S_E);
      } else {
        uint8_t l = ErrorRx::getReceivedDataLength();
        uint16_t v = ErrorRx::getDataByte(0);
        if (l > 1) {
          uint8_t b1 = ErrorRx::getDataByte(1);
          b1 >>= 4;
          v <<= 4;
          v |= b1;
        }
        KH2441EF::setDisplayBufToIntAsHex(v, false);
      }
    }
  }
}
