#include <Arduino.h>
#include "rs485_client.h"
#include "tm1637.h"
#include "input_button.h"

int pin_RS485_dir = 2; // LOW - Listening, HIGH - Transmitting
int pin_TM1637_CLK = 4;
int pin_TM1637_DIO = 5;

InputButton::Def btnPlus = { .pinNo = 6, .isActiveHigh = false, .enablePoolup = true };
InputButton::Def btnMinus = { .pinNo = 7, .isActiveHigh = false, .enablePoolup = true };
InputButton::Def btnPower = { .pinNo = 8, .isActiveHigh = false, .enablePoolup = true };

bool powerOnRequest = false;
bool fanOnRequest = false;
bool heatRequest = false;
bool fireAlarm = false; // in case fire is registered!

unsigned int digitsDisplayValue = 0;
bool digitsDisplayShowDoubleDots = false;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(38400);
  RS485Client::init(pin_RS485_dir);
  TM1637::init(pin_TM1637_CLK, pin_TM1637_DIO);
  TM1637::updateDisplayWithError(1);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(2000);
  TM1637::updateDisplay(digitsDisplayValue, digitsDisplayShowDoubleDots);
  digitalWrite(LED_BUILTIN, LOW);
}

bool btnPlusLastSaved = true;
bool btnMinusLastSaved = true;
bool btnPowerLastSaved = true;
int j = 100;

uint8_t _o_b = 0;
uint8_t _o_d = 0;
void loop() {
  RS485Client::loop();
  //testLoop();

  switch (_o_b) { // loop only one button at a time to make RS485 loop more efficient
    case 0: InputButton::loopFor(btnPlus); break;
    case 1: InputButton::loopFor(btnMinus); break;
    case 2: InputButton::loopFor(btnPower); break;
    default: _o_b = 0;
  } _o_b++; _o_b = _o_b % 3;

  if (btnPlusLastSaved != InputButton::isPressed(btnPlus)) {
    btnPlusLastSaved = !btnPlusLastSaved;
    if (btnPlusLastSaved) j++;
    if (j > 83) heatRequest = false;
    RS485Client::updateFlags(powerOnRequest, fanOnRequest, heatRequest, fireAlarm);
  }
  if (btnMinusLastSaved != InputButton::isPressed(btnMinus)) {
    btnMinusLastSaved = !btnMinusLastSaved;
    if (btnMinusLastSaved) j--;
    if (j < 80) heatRequest = true;
    RS485Client::updateFlags(powerOnRequest, fanOnRequest, heatRequest, fireAlarm);
  }
  if (btnPowerLastSaved != InputButton::isPressed(btnPower)) {
    btnPowerLastSaved = !btnPowerLastSaved;
    digitsDisplayShowDoubleDots = btnPowerLastSaved; //TM1637::updateDisplay(j, btnPowerLastSaved);
    powerOnRequest = btnPowerLastSaved;
    RS485Client::updateFlags(powerOnRequest, fanOnRequest, heatRequest, fireAlarm);
  }

  if (++_o_d >= 5) {
    digitsDisplayValue = j;
    loop_digitsDisplay();
    _o_d = 0;
  }
}

uint8_t state_tempSensors = 0;

void loop_tempSensors() {
}

int digitsDisplayValue_displayed = digitsDisplayValue;
bool digitsDisplayShowDoubleDots_displayed = digitsDisplayShowDoubleDots;
void loop_digitsDisplay() {
  if (digitsDisplayValue != digitsDisplayValue_displayed || digitsDisplayShowDoubleDots != digitsDisplayShowDoubleDots_displayed) {
    TM1637::updateDisplay(digitsDisplayValue, digitsDisplayShowDoubleDots, false, digitsDisplayValue%7);
    digitsDisplayValue_displayed = digitsDisplayValue;
    digitsDisplayShowDoubleDots_displayed = digitsDisplayShowDoubleDots;
  }
}

void loop_leds() {
}
