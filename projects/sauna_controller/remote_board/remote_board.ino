#include <Arduino.h>
#include "rs485_client.h"
#include "tm1637.h"
#include "input_button.h"

int pin_RS485_dir = 2; // LOW - Listening, HIGH - Transmitting
int pin_TM1637_CLK = 4;
int pin_TM1637_DIO = 5;

InputButton::Def btnPlus = {6, false, true};
//InputButton::Def btnPlus = { .pinNo = 6, .isActiveHigh = false, .enablePoolup = true };
InputButton::Def btnMinus = { .pinNo = 7, .isActiveHigh = false, .enablePoolup = true };
InputButton::Def btnPower = { .pinNo = 8, .isActiveHigh = false, .enablePoolup = true };

bool powerOnRequest = false;
bool fanOnRequest = false;
bool heatRequest = false;
bool fireAlarm = false; // in case fire is registered!

unsigned int digitsDisplayValue = 0;
bool digitsDisplayShowDoubleDots = false;

void setup() {
  Serial.begin(38400);
  RS485Client::init(pin_RS485_dir);
  TM1637::init(pin_TM1637_CLK, pin_TM1637_DIO);
  TM1637::updateDisplay(1234, true);
  delay(2000);
  TM1637::updateDisplay(digitsDisplayValue, digitsDisplayShowDoubleDots);
}

bool btnPowerLastSaved = false;
void loop() {
  RS485Client::loop();
  testLoop();

  //InputButton::loopFor(btnPlus);
  //InputButton::loopFor(btnMinus);
  InputButton::loopFor(btnPower);

  if (btnPowerLastSaved != InputButton::isPressed(btnPower)) {
    btnPowerLastSaved = !btnPowerLastSaved;
    TM1637::updateDisplay(digitsDisplayValue, btnPowerLastSaved);
  }

  if (InputButton::isError(btnPower)) {
    TM1637::updateDisplay(9999, true);
    delay(500);
    TM1637::updateDisplay(digitsDisplayValue, digitsDisplayShowDoubleDots);
  }
}

int i = -1;
unsigned long timerMark = millis();
void testLoop() {
  if (millis() - timerMark > 20000) {
    timerMark = millis();
    i++;
    powerOnRequest = false; fanOnRequest = false; heatRequest = false; fireAlarm = false;
    if (i % 4 == 0) powerOnRequest = true;
    else if (i % 4 == 1) fanOnRequest = true;
    else if (i % 4 == 2) heatRequest = true;
    else if (i % 4 == 3) fireAlarm = true;
    RS485Client::updateFlags(powerOnRequest, fanOnRequest, heatRequest, fireAlarm);
  }
}

uint8_t state_tempSensors = 0;

void loop_tempSensors() {
}

void loop_buttons() {
}

int digitsDisplayValue_displayed = digitsDisplayValue;
bool digitsDisplayShowDoubleDots_displayed = digitsDisplayShowDoubleDots;
void loop_digitsDisplay() {
  if (digitsDisplayValue != digitsDisplayValue_displayed || digitsDisplayShowDoubleDots != digitsDisplayShowDoubleDots_displayed) {
    TM1637::updateDisplay(digitsDisplayValue, digitsDisplayShowDoubleDots);
    digitsDisplayValue_displayed = digitsDisplayValue;
    digitsDisplayShowDoubleDots_displayed = digitsDisplayShowDoubleDots;
  }
}

void loop_leds() {
}
