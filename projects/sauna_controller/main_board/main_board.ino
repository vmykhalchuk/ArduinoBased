#include <Arduino.h>
#include "switch_pin.h"
#include "rs485_server.h"
#include "info_panel.h"
#include "ds18b20.h"
#include "tests.h"

const int pin_RS485_dir = 2; // LOW - Listening, HIGH - Transmitting

// 4 - External
// 5 - Internal (inside enclosure)
// 6-8 - TRIACs
const int TEMP_SENS_COUNT = 5;
const int pins_DS18B20[TEMP_SENS_COUNT] = { 4, 5, 6, 7, 8};

const SwitchDef sw_fan_TRIACs = {10, IS_ACTIVE_LOW};
const SwitchDef sw_fan_Main = {11, IS_ACTIVE_HIGH};
const SwitchDef sw_Heater_TRIACs = {12, IS_ACTIVE_HIGH}; // activates all three TRIACs of Heater

const int pin_TestBtn = 13; // LOW - Pressed

// Main power stage (Contactor)
const SwitchDef sw_Relay1_ALARM = {A0, IS_ACTIVE_LOW};
// Alarm (Relay1 must be off for this to work!)
const SwitchDef sw_Relay2_HEAT_FAN = {A1, IS_ACTIVE_LOW};
// Fan output (AC 220V Heating fan)
const SwitchDef sw_Relay3_POWER = {A2, IS_ACTIVE_LOW};
// Reserved for future use
const SwitchDef sw_Relay4 = {A3, IS_ACTIVE_LOW};

const SwitchDef sw_InfoPanel_Led1 = {A4, IS_ACTIVE_HIGH};
const SwitchDef sw_InfoPanel_Buzzer = {A5, IS_ACTIVE_HIGH};

// Remote flags (data received from Remote)
bool powerOnRequest = false;
bool fanOnRequest = false;
bool heatRequest = false;
bool fireAlarm = false; // in case fire is registered!

bool isSystemPoweredOn = false;
unsigned long systemPowerOnTimerMark = 0;
unsigned long dataReceivedTimerMark = 0;

float temperatures[TEMP_SENS_COUNT];

void setup() {
  Serial.begin(38400);
  RS485Server::init(pin_RS485_dir);
  InfoPanel::init(sw_InfoPanel_Led1, sw_InfoPanel_Buzzer);
  
  initSwitch(sw_fan_TRIACs);
  initSwitch(sw_fan_Main, true);
  initSwitch(sw_Heater_TRIACs);

  initSwitch(sw_Relay1_ALARM);
  initSwitch(sw_Relay2_HEAT_FAN);
  initSwitch(sw_Relay3_POWER);
  initSwitch(sw_Relay4);

  initSwitch(sw_InfoPanel_Led1);
  initSwitch(sw_InfoPanel_Buzzer, true);
  
  pinMode(pin_TestBtn, INPUT);
  delay(20);
  bool isRunTests = false;
  if (digitalRead(pin_TestBtn) == HIGH) {
    isRunTests = true;
  }
  digitalWrite(pin_TestBtn, LOW);
  pinMode(pin_TestBtn, OUTPUT);
  delay(2000);
  switchOff(sw_fan_Main);
  switchOff(sw_InfoPanel_Buzzer);
  dataReceivedTimerMark = millis();

  if (isRunTests) {
    while (true) {
       delay(2000);
       Tests::runTests(sw_InfoPanel_Buzzer, sw_fan_Main, sw_fan_TRIACs, sw_Heater_TRIACs,
                       sw_Relay1_ALARM, sw_Relay2_HEAT_FAN, sw_Relay3_POWER, sw_Relay4); 
    }
  }

  
  // init temp sensors
  bool allSensorsPresent = true;
  for (int i = 0; i < 1/*FIXME TEMP_SENS_COUNT*/; i++) {
    allSensorsPresent &= DS18B20::setResolution(pins_DS18B20[i], 0x3F);
    temperatures[i] = DS18B20::readTemperature(pins_DS18B20[i]);
  }

  if (!allSensorsPresent) {
    // critical error, we cannot continue operation
    fullStopBecauseOfCriticalError(3);
  }
  
}

void powerSystemOn() {
  if (isSystemPoweredOn) return;
  if (fireAlarm) return; // for safety reasons we do not let system on!

  // toggle Alarm for 3 sec to test it works!
  switchOff(sw_Relay3_POWER);
  switchOn(sw_Relay1_ALARM);
  delay(3000);
  switchOff(sw_Relay1_ALARM);
  switchOn(sw_Relay3_POWER);

  isSystemPoweredOn = true;
  systemPowerOnTimerMark = millis();
}

void powerSystemOff() {
  switchOff(sw_Heater_TRIACs);
  switchOff(sw_Relay3_POWER);
  //switchOff(sw_Relay1_ALARM); DO NOT SWITCH IT OFF IF ALREADY ON!!!
  switchOff(sw_Relay2_HEAT_FAN);
  switchOff(sw_Relay4);

  isSystemPoweredOn = false;
}

void fullStopBecauseOfCriticalError(int blinkError) {
  powerSystemOff();
  digitalWrite(LED_BUILTIN, HIGH);
  while (true) {
    delay(3000);
    blink(sw_InfoPanel_Buzzer, blinkError, 700, 300);
  }
}

void loop() {
  RS485Server::loop();
  RS485Server::Error rs485Error = RS485Server::popError();
  if (rs485Error != RS485Server::OK) {
    InfoPanel::setCommunicationError();
    if (rs485Error == RS485Server::BAD_CRC) {
      blink(sw_InfoPanel_Buzzer, 1, 100); delay(2000);
    } else if (rs485Error == RS485Server::BAD_DATA) {
      blink(sw_InfoPanel_Buzzer, 2, 100); delay(2000);
    } else if (rs485Error == RS485Server::NOT_ENOUGH_BYTES_RECEIVED) {
      blink(sw_InfoPanel_Buzzer, 3, 100); delay(2000);
    }
  }
  if (RS485Server::popDataRefreshedFlag()) {
    InfoPanel::clearCommunicationError();
    handleRS485DataRefreshed();
  }

  if ((millis() - dataReceivedTimerMark) > 10000) {
    if (isSystemPoweredOn) {
      // remote board is unavailable while System Power is ON - we must turn it OFF for safety!!!
      powerSystemOff();
    }
    InfoPanel::setCommunicationError();
    blink(sw_InfoPanel_Buzzer, 3, 50);
    dataReceivedTimerMark = millis();
  }
  InfoPanel::loop();
}


void handleRS485DataRefreshed() {
  dataReceivedTimerMark = millis();

  if (false) {
    Tests::testDataRefresh(sw_InfoPanel_Buzzer, sw_Relay1_ALARM, sw_Relay2_HEAT_FAN, sw_Relay3_POWER, sw_Relay4);
    return;
  }
  
  if (RS485Server::f4) fireAlarm = true; // will not reset unless full system reset!
  
  if (fireAlarm) {
    powerSystemOff();
    switchOn(sw_Relay1_ALARM); // switch Alarm!!!
    return; // no more actions allowed!!!
  }

  if (powerOnRequest != RS485Server::f1) {
    powerOnRequest = RS485Server::f1;
    if (powerOnRequest) {
      powerSystemOn();
    } else {
      powerSystemOff();
    }
  }

  if (fanOnRequest != RS485Server::f2) {
    fanOnRequest = RS485Server::f2;
    switchToggleTo(sw_Relay2_HEAT_FAN, fanOnRequest);
  }

  if (heatRequest != RS485Server::f3) {
    heatRequest = RS485Server::f3;
    switchToggleTo(sw_Heater_TRIACs, heatRequest);
  }
}
