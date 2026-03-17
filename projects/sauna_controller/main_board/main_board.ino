#include <Arduino.h>
#include "switch_pin.h"
#include "rs485_server.h"
#include "tests.h"

int pin_RS485_dir = 2; // LOW - Listening, HIGH - Transmitting

// 4 - External
// 5 - Internal (inside enclosure)
// 6-8 - TRIACs
int pins_DS18B20[] = { 4, 5, 6, 7, 8};

SwitchDef sw_fan_TRIACs = {10, IS_ACTIVE_LOW};
SwitchDef sw_fan_Main = {11, IS_ACTIVE_HIGH};
SwitchDef sw_Heater = {12, IS_ACTIVE_HIGH}; // activates all three TRIACs

int pin_TestBtn = 13; // LOW - Pressed

// Main power stage (Contactor)
SwitchDef sw_Relay1_POWER = {A0, IS_ACTIVE_LOW};
// Alarm (Relay1 must be off for this to work!)
SwitchDef sw_Relay2_ALARM = {A1, IS_ACTIVE_LOW};
// Fan output (AC 220V Heating fan)
SwitchDef sw_Relay3_HEAT_FAN = {A2, IS_ACTIVE_LOW};
// Reserved for future use
SwitchDef sw_Relay4 = {A3, IS_ACTIVE_LOW};

SwitchDef sw_Panel_Led1 = {A6, IS_ACTIVE_HIGH};
SwitchDef sw_Panel_Buzzer = {A7, IS_ACTIVE_HIGH};

// Remote flags
bool powerOnRequest = false;
bool fanOnRequest = false;
bool heatRequest = false;
bool fireAlarm = false; // in case fire is registered!

void setup() {
  RS485Server::init(pin_RS485_dir);
  Tests::init(sw_Panel_Led1, sw_Panel_Buzzer);
  Serial.begin(38400);
  initSwitch(sw_fan_TRIACs);
  initSwitch(sw_fan_Main);
  initSwitch(sw_Heater);

  initSwitch(sw_Relay1_POWER);
  initSwitch(sw_Relay2_ALARM);
  initSwitch(sw_Relay3_HEAT_FAN);
  initSwitch(sw_Relay4);

  initSwitch(sw_Panel_Led1);
  initSwitch(sw_Panel_Buzzer);
  
  pinMode(pin_TestBtn, INPUT);
  if (digitalRead(pin_TestBtn) == LOW) {
    runTests();
  }
  pinMode(pin_TestBtn, OUTPUT);
}

void runTests() {
  for (uint8_t i = 1; i <= 5; i++) {
    Tests::blinkTestNo(i);
    switch (i) {
      case 1: 
        delay(1000);
        switchOn(sw_fan_Main);
        delay(3000);
        switchOff(sw_fan_Main);
        break;
      case 2:
        delay(1000);
        switchOn(sw_fan_TRIACs);
        delay(3000);
        switchOff(sw_fan_TRIACs);
        break;
      case 3:
        delay(1000);
        switchOn(sw_Heater);
        delay(3000);
        switchOff(sw_Heater);
        break;
      case 4:
        delay(1000);
        switchOn(sw_Relay1_POWER); switchOn(sw_Relay2_ALARM);
        delay(3000);
        switchOff(sw_Relay1_POWER); switchOff(sw_Relay2_ALARM);
        break;
      case 5:
        delay(1000);
        switchOn(sw_Relay3_HEAT_FAN); switchOn(sw_Relay4);
        delay(3000);
        switchOff(sw_Relay3_HEAT_FAN); switchOff(sw_Relay4);
        break;
    }

    delay(2000);
  }
}


void powerSystemOn() {
  if (fireAlarm) return; // for safety reasons we do not let system on!

  // toggle Alarm for 3 sec to test it works!
  switchOff(sw_Relay1_POWER);
  switchOn(sw_Relay2_ALARM);
  delay(3000);
  switchOff(sw_Relay2_ALARM);
  switchOn(sw_Relay1_POWER);
}

void powerSystemOff() {
  switchOff(sw_Heater);
  switchOff(sw_Relay1_POWER);
  //switchOff(sw_Relay2_ALARM); DO NOT SWITCH IT OFF IF ALREADY ON!!!
  switchOff(sw_Relay3_HEAT_FAN);
  switchOff(sw_Relay4);
}

void loop() {
  RS485Server::loop();
  if (RS485Server::errorCode != RS485Server::OK) {
    // Handle error!
    digitalWrite(LED_BUILTIN, HIGH);
  }
  
  bool powerOnRequestChanged = false;
  bool fanOnRequestChanged = false;
  bool heatRequestChanged = false;
  
  if (RS485Server::flagsUpdated) {
    if (RS485Server::f4) fireAlarm = true; // will not reset unless full system reset!
    
    powerOnRequestChanged = powerOnRequest != RS485Server::f1;
    fanOnRequestChanged = fanOnRequest != RS485Server::f2;
    heatRequestChanged = heatRequest != RS485Server::f3;

    powerOnRequest = RS485Server::f1;
    fanOnRequest = RS485Server::f2;
    heatRequest = RS485Server::f3;
  }

  if (fireAlarm) {
    powerSystemOff();
    switchOn(sw_Relay2_ALARM); // switch Alarm!!!
    return; // no more actions allowed!!!
  }

  if (powerOnRequestChanged) {
    if (powerOnRequest) {
      powerSystemOn();
    } else {
      powerSystemOff();
    }
  }

  if (fanOnRequestChanged) {
    if (fanOnRequest) {
      switchOn(sw_Relay3_HEAT_FAN);
    } else {
      switchOff(sw_Relay3_HEAT_FAN);
    }
  }

  if (heatRequestChanged) {
    if (heatRequest) {
      switchOn(sw_Heater);
    } else {
      switchOff(sw_Heater);
    }
  }
}
