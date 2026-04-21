#include <Arduino.h>
#include "clock.h"
#include "rs485_client.h"
#include "tm1637.h"
#include "input_button.h"

int pin_RS485_DIR = 2; // LOW - Listening, HIGH - Transmitting
int pin_TM1637_CLK = 4;
int pin_TM1637_DIO = 5;

InputButton::Def btnPlus = { .pinNo = 6, .isActiveHigh = false, .enablePullup = true };
InputButton::Def btnMinus = { .pinNo = 7, .isActiveHigh = false, .enablePullup = true };
InputButton::Def btnPower = { .pinNo = 8, .isActiveHigh = false, .enablePullup = true };

RS485Client::OutputData _out = {
                                  .powerOnRequest = false,
                                  .fanOnRequest = false,
                                  .heatRequest = false,
                                  .fireAlarm = false
                               };
unsigned int display_value = 0;
bool display_doubleDots = false;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(38400);
  RS485Client::init(pin_RS485_DIR, _out);
  TM1637::init(pin_TM1637_CLK, pin_TM1637_DIO);
  TM1637::updateDisplayWithError(3);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(2000);
  TM1637::updateDisplay(display_value, display_doubleDots);
  digitalWrite(LED_BUILTIN, LOW);
}

bool btnPlusLastState = true;
bool btnMinusLastState = true;
bool btnPowerLastState = true;
int j = 85;

uint8_t _c_b = 0;
uint8_t _c_d = 0;
void loop() {
  ClockLR::tick();
  RS485Client::tick();

  switch (_c_b) { // tick only one button at a time to make RS485 handling more smooth
    case 0: InputButton::tick(btnPlus); break;
    case 1: InputButton::tick(btnMinus); break;
    case 2: InputButton::tick(btnPower); break;
    default: _c_b = 0;
  } _c_b++; _c_b = _c_b % 3;

  if (InputButton::wasPressed(btnPlus)) {
    j++;
    if (j > 81) _out.heatRequest = false;
  }
  if (InputButton::wasPressed(btnMinus)) {
    j--;
    if (j < 80) _out.heatRequest = true;
  }
  if (InputButton::hasStateChanged(btnPower)) {
    _out.powerOnRequest = InputButton::isPressed(btnPower);
    display_doubleDots = _out.powerOnRequest;
  }

  if (++_c_d >= 5) {
    display_value = j;
    tick_display();
    _c_d = 0;
  }
}

uint8_t state_tempSensors = 0;

void tick_tempSensors() {
  // FIXME Implement it!
}

int _display_value_displayed = display_value;
bool _display_doubleDots_displayed = display_doubleDots;
void tick_display() {
  if (display_value != _display_value_displayed || display_doubleDots != _display_doubleDots_displayed) {
    TM1637::updateDisplay(display_value, display_doubleDots);
    _display_value_displayed = display_value;
    _display_doubleDots_displayed = display_doubleDots;
  }
}
