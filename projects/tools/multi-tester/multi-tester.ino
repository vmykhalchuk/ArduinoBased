#include <Arduino.h>
#include "adc_protected.h"
#include "clock.h"
#include "ds18b20.h"
#include "htu21d.h"
#include "input_button.h"
#include "kh2441ef.h"

InputButton::Def btnMain = { .pinNo = 3, .isActiveHigh = false, .enablePullup = true };

int tempSensorDS18B20Pin = 4;


uint8_t progNo = 0;
uint16_t mainStartMs;

void setup() {
  mainStartMs = ClockLR::now;
  while (!ClockLR::isElapsed(mainStartMs, InputButton::LONG_PRESS_DURATION_MS << 1)) {
    ClockLR::tick();
    InputButton::tick(btnMain);
    if (InputButton::isLongPressed(btnMain)) {
      // enter select program mode
      selectProgramMode();
      break;
    }
  };
  
  HTU21D::setup();
  mainStartMs = ClockLR::now;
}

enum SelProgState { NOT_INTLZD, IDLE, WAITING_4DOUBLE_CLICK, WAITING_4EXIT };
void selectProgramMode() {
  bool isOn = false;
  SelProgState state = NOT_INTLZD;
  KH2441EF::setDisplayBuf(0, 0, 0x13, 0, false);
  while (true) {
    ClockLR::tick();
    InputButton::tick(btnMain);
    KH2441EF::tick();
    if (ClockLR::isElapsed(mainStartMs, 500)) {
      mainStartMs = ClockLR::now;
      // update screen
      updateDisplayWithProgNo(isOn);
      isOn = !isOn;
    }
    bool _exit = false;
    switch (state) {
      case NOT_INTLZD:
        if (!InputButton::isLongPressed(btnMain)) {
          state = IDLE;
          InputButton::wasPressed(btnMain); // erase wasPressed flag (stays on after longPressed)
          InputButton::wasReleased(btnMain); // this is hack
        }
      break;
      case IDLE:
        if (InputButton::wasReleased(btnMain)) {
          mainStartMs = ClockLR::now;
          state = WAITING_4DOUBLE_CLICK;
        }
        if (InputButton::isLongPressed(btnMain)) {
          state = WAITING_4EXIT;
        }
      break;
      case WAITING_4DOUBLE_CLICK:
        if (InputButton::wasReleased(btnMain)) {
          // DoubleClick
          incrementProgNo(true);
          updateDisplayWithProgNo(isOn);
          state = IDLE;
        }
        if (ClockLR::isElapsed(mainStartMs, 1000)) {
          // No DoubleClick
          incrementProgNo(false);
          updateDisplayWithProgNo(isOn);
          state = IDLE;
        }
      break;
      case WAITING_4EXIT:
        if (!InputButton::isPressed(btnMain)) {
          InputButton::wasPressed(btnMain); // FIXME This is hack to clear flags
          InputButton::wasReleased(btnMain); // FIXME This is hack to clear flags
          _exit = true;
        }
      break;
    }
    if (_exit) break;
  }
}

void updateDisplayWithProgNo(bool isOn) {
  KH2441EF::setDisplayBuf(0, 0x13, isOn ? progNo/10 : 0x10, isOn ? progNo%10 : 0x10, false);
}

void incrementProgNo(bool doubleClick) {
  uint8_t maj = progNo / 10, minor = progNo % 10;
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
      case 1: minor = minor >= 2 ? 0 : minor+1; break;
      case 2: minor = minor >= 3 ? 0 : minor+1; break;
      case 9: minor = minor >= 9 ? 9 : minor+1; break;
      default: maj = 0; minor = 0;
    }
  }
  progNo = maj * 10 + minor;
}

uint8_t i = 0;
uint8_t progsData[] = {0,0};
bool _firstRun = true;
void loop() {
  ClockLR::tick();
  InputButton::tick(btnMain);
  KH2441EF::tick();
  
  switch(progNo) {
    case 0: tickProg00(progsData[0], progsData[1]); break;
    case 3: tickProg03(progsData[0], progsData[1]); break;
    default:
      KH2441EF::setDisplayBufToErrorMsg();
  }

  if (_firstRun) _firstRun = false;
}

void tickProgTT() {
  if (ClockLR::isElapsed(mainStartMs, 1000)) {
    mainStartMs = ClockLR::now;
    if (i < 10) {
      if (i%2 == 0) KH2441EF::setDisplayBufToErrorMsg();
      else KH2441EF::clearDisplayBuf();
    } else {
      KH2441EF::clearDisplayBuf();
      KH2441EF::setDisplayBuf(1,2,3,4,i%2);
    }
    i++;
  }
}

void tickProg00(uint8_t &pd0, uint8_t &wasBtnPressed) {
  bool forceDataRead = false;
  if (_firstRun || InputButton::wasPressed(btnMain)) {
    wasBtnPressed = true;
    mainStartMs = ClockLR::now;
    KH2441EF::setDisplayBuf(KH2441EF::S_BLANK, KH2441EF::S_t, KH2441EF::S_UND, KH2441EF::S_C, false);
  }
  if (wasBtnPressed && ClockLR::isElapsed(mainStartMs, 600)) {
    wasBtnPressed = false;
    forceDataRead = true;
  }
  if (forceDataRead || ClockLR::isElapsed(mainStartMs, 10000)) {
    KH2441EF::muteDisplayInstantly(); // to prevent some segments to light bright while Temp measurements take place
    float t = DS18B20::readTemperature(tempSensorDS18B20Pin);
    KH2441EF::setDisplayBufToFloatWithOneDecimal(t);
    mainStartMs = ClockLR::tick();
  }
}

void tickProg01(uint8_t &pd0Mode, uint8_t &wasBtnPressed) {
  bool forceDataRead = false;
  if (_firstRun || InputButton::wasPressed(btnMain)) {
    wasBtnPressed = true;
    pd0Mode = pd0Mode == 0 ? 1 : 0;
    mainStartMs = ClockLR::now;
    if (pd0Mode == 0) {
      KH2441EF::setDisplayBuf(KH2441EF::S_BLANK, KH2441EF::S_t, KH2441EF::S_UND, KH2441EF::S_C, false);
    } else {
      KH2441EF::setDisplayBuf(KH2441EF::S_BLANK, KH2441EF::S_BLANK, KH2441EF::S_r, KH2441EF::S_h, false);
    }
  }
  if (wasBtnPressed && ClockLR::isElapsed(mainStartMs, 600)) {
    wasBtnPressed = false;
    forceDataRead = true;
  }
  if (forceDataRead || ClockLR::isElapsed(mainStartMs, 10000)) {
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
    mainStartMs = ClockLR::tick();
  }
}

void tickProg03(uint8_t &pd0, uint8_t &pd1) {
  if (ClockLR::isElapsed(mainStartMs, 1000)) {
    KH2441EF::muteDisplayInstantly(); // to prevent some segments to light bright while ADC conversion happens
    uint16_t v = AdcProtected::protectedAnalogRead(A0);
    KH2441EF::setDisplayBufToInt(v);
    mainStartMs = ClockLR::tick();
  }
}
