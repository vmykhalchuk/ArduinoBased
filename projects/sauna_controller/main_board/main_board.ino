#include <Arduino.h>
#include "switch_pin.h"
#include "rs485_server.h"
#include "info_panel.h"
#include "ds18b20.h"
#include "tests.h"

const int pin_RS485_dir = 2; // LOW - Listening, HIGH - Transmitting

const float OVERHEAT_TEMPERATURE = 50; // When overheat occurs - emergency shutdown is executed!
const float MAX_TRIAC_FANS_TEMPERATURE = 30;
const float MIN_TRIAC_FANS_TEMPERATURE = 25;
const float DELTA_TEMP = 1.5;

DS18B20::TempSensorsStruct tempSensors = {
          .triac1 = { .pinNo = 4, DS18B20::NO_READING },
          .triac2 = { .pinNo = 5, DS18B20::NO_READING },
          .triac3 = { .pinNo = 6, DS18B20::NO_READING },
          .internal = { .pinNo = 7, DS18B20::NO_READING },
          .external = { .pinNo = 8, DS18B20::NO_READING }
        };
const uint8_t TEMP_SENSORS_COUNT = 5;
DS18B20::TempSensorDef* allTempSensors[TEMP_SENSORS_COUNT] = {
  &tempSensors.triac1, &tempSensors.triac2, &tempSensors.triac3,
  &tempSensors.internal, &tempSensors.external
};

SwitchDef sw_fan_TRIACs = {10, IS_ACTIVE_LOW};
SwitchDef sw_fan_Main = {11, IS_ACTIVE_HIGH};
SwitchDef sw_Heater_TRIACs = {12, IS_ACTIVE_HIGH}; // activates all three TRIACs of Heater

const int pin_TestBtn = 13; // LOW - Pressed

// Main power stage (Contactor)
SwitchDef sw_Relay1_ALARM = {A0, IS_ACTIVE_LOW};
// Alarm (Relay1 must be off for this to work!)
SwitchDef sw_Relay2_HEAT_FAN = {A1, IS_ACTIVE_LOW};
// Fan output (AC 220V Heating fan)
SwitchDef sw_Relay3_POWER = {A2, IS_ACTIVE_LOW};
// Reserved for future use
SwitchDef sw_Relay4 = {A3, IS_ACTIVE_LOW};

SwitchDef sw_InfoPanel_Led1 = {A4, IS_ACTIVE_HIGH};
SwitchDef sw_InfoPanel_Buzzer = {A5, IS_ACTIVE_HIGH};

// Remote flags (data received from Remote)
RS485Server::InputData rs485Input = {
                                        .powerOnRequest = false,
                                        .fanOnRequest = false,
                                        .heatRequest = false,
                                        .fireAlarm = false
                                      };
RS485Server::InputData rs485InputStored = {
                                        .powerOnRequest = false,
                                        .fanOnRequest = false,
                                        .heatRequest = false,
                                        .fireAlarm = false
                                      };

bool isSystemPoweredOn = false;
unsigned long systemPowerOnTimeMark = 0;
unsigned long dataReceivedTimeMark = 0;

bool _isTestMode = false;

void setup() {
  Serial.begin(38400);
  
  initSwitch(sw_fan_Main, true);
  initSwitch(sw_fan_TRIACs);
  initSwitch(sw_Heater_TRIACs);

  initSwitch(sw_Relay1_ALARM);
  initSwitch(sw_Relay2_HEAT_FAN);
  initSwitch(sw_Relay3_POWER);
  initSwitch(sw_Relay4);

  initSwitch(sw_InfoPanel_Led1);
  initSwitch(sw_InfoPanel_Buzzer, true);
  
  RS485Server::init(pin_RS485_dir, rs485Input);
  InfoPanel::init(sw_InfoPanel_Led1, sw_InfoPanel_Buzzer);

  pinMode(pin_TestBtn, INPUT);
  delay(20);
  if (digitalRead(pin_TestBtn) == HIGH) {
    _isTestMode = true;
  }
  digitalWrite(pin_TestBtn, LOW);
  delay(2000);
  switchOff(sw_fan_Main);
  switchOff(sw_InfoPanel_Buzzer);

  if (_isTestMode) {
     Tests::runTests(sw_InfoPanel_Buzzer, sw_fan_Main, sw_fan_TRIACs, sw_Heater_TRIACs,
                     sw_Relay1_ALARM, sw_Relay2_HEAT_FAN, sw_Relay3_POWER, sw_Relay4, pin_TestBtn);
  }

  initTempSensors();

  pinMode(pin_TestBtn, OUTPUT);
  while (Serial.available()) Serial.read();
  dataReceivedTimeMark = millis();
}

void initTempSensors() {
  // init temp sensors
  bool allSensorsPresent = true;

  allSensorsPresent &= DS18B20::setResolution(tempSensors.triac1.pinNo, 0x3F);
  if (false) { //FIXME Enable
  allSensorsPresent &= DS18B20::setResolution(tempSensors.triac2.pinNo, 0x3F);
  allSensorsPresent &= DS18B20::setResolution(tempSensors.triac3.pinNo, 0x3F);
  allSensorsPresent &= DS18B20::setResolution(tempSensors.internal.pinNo, 0x3F);
  allSensorsPresent &= DS18B20::setResolution(tempSensors.external.pinNo, 0x3F);
  }
  
  if (!allSensorsPresent) {
    // critical error, we cannot continue operation, some of temp sensors fails
    emergencyShutdown(1, false);
  }

  DS18B20::readTemperature(tempSensors.triac1);
  if (false) { //FIXME Enable
  DS18B20::readTemperature(tempSensors.triac2);
  DS18B20::readTemperature(tempSensors.triac3);
  DS18B20::readTemperature(tempSensors.internal);
  DS18B20::readTemperature(tempSensors.external);
  }
}

void powerSystemOn() {
  if (isSystemPoweredOn) return;

  if (isSwitchOn(sw_Heater_TRIACs)) {
    switchOff(sw_Heater_TRIACs);
    _delay(1000);
  }
  switchOff(sw_Relay3_POWER);
  
  // toggle Alarm for 3 sec to test it works!
  switchOn(sw_Relay1_ALARM);
  _delay(3000);
  switchOff(sw_Relay1_ALARM);
  switchOn(sw_Relay3_POWER);

  isSystemPoweredOn = true;
  systemPowerOnTimeMark = millis();
}

void powerSystemOff(bool activeDelay = true) {
  switchOff(sw_Heater_TRIACs);
  
  // we must keep delay between TRIACs and Contactor's transitioning
  if (activeDelay) _delay(2000); else delay(2000);
  
  switchOff(sw_Relay3_POWER);
  //switchOff(sw_Relay1_ALARM); DO NOT SWITCH IT OFF IF ALREADY ON!!!
  switchOff(sw_Relay2_HEAT_FAN);
  switchOff(sw_Relay4);

  isSystemPoweredOn = false;
}

void emergencyShutdown(int blinkError, bool enableFireAlarm) {
  powerSystemOff(false);
  if (enableFireAlarm) switchOn(sw_Relay1_ALARM);
  digitalWrite(LED_BUILTIN, HIGH);
  while (true) {
    delay(5000);
    blink(sw_InfoPanel_Buzzer, blinkError, 700, 300);
  }
}

void _delay(uint16_t timeMs) {
  uint32_t startMs = millis();
  while (millis() - startMs < timeMs) {
    _loopWithoutActions();
  }
}

void _loopWithoutActions() {
  InfoPanel::loop();
  loopTempSensors();
  RS485Server::loop();
}

void loop() {
  _loopWithoutActions();
  RS485Server::Error rs485Error = RS485Server::popError();
  if (rs485Error != RS485Server::OK) {
    InfoPanel::setCommunicationError();
    if (_isTestMode) {
      if (rs485Error == RS485Server::BAD_CRC) {
        blink(sw_InfoPanel_Buzzer, 1, 100); _delay(2000);
      } else if (rs485Error == RS485Server::BAD_DATA) {
        blink(sw_InfoPanel_Buzzer, 2, 100); _delay(2000);
      } else if (rs485Error == RS485Server::NOT_ENOUGH_BYTES_RECEIVED) {
        blink(sw_InfoPanel_Buzzer, 3, 100); _delay(2000);
      }
    }
  }
  if (RS485Server::popDataRefreshedFlag()) {
    InfoPanel::clearCommunicationError();
    handleRS485DataRefreshed();
  }

  if ((millis() - dataReceivedTimeMark) > 10000) {
    if (isSystemPoweredOn) {
      // remote board is unavailable while System Power is ON - we must turn it OFF for safety!!!
      powerSystemOff();
    }
    InfoPanel::setCommunicationError();
    blink(sw_InfoPanel_Buzzer, 3, 100);//FIXME Move into InfoPanel::loop
    dataReceivedTimeMark = millis();
  }
}

uint8_t _tempSensorIdx = 0;
uint32_t _tempSensorLastReadTime = 0;
const uint16_t TEMP_SENSOR_READ_INTERVAL_MS = 1000; // ms between different sensor reads

void loopTempSensors() {
  if (millis() - _tempSensorLastReadTime >= TEMP_SENSOR_READ_INTERVAL_MS) {
    _tempSensorLastReadTime = millis();
    DS18B20::readTemperature(*allTempSensors[_tempSensorIdx]);
    handleTempsUpdated();
    _tempSensorIdx++;
    if (_tempSensorIdx >= TEMP_SENSORS_COUNT) _tempSensorIdx = 0;
  }
}

void handleTempsUpdated() {
  bool criticalTemp = (tempSensors.triac1.temp > OVERHEAT_TEMPERATURE);// || (tempSensors.triac2.temp > OVERHEAT_TEMPERATURE) || (tempSensors.triac3.temp > OVERHEAT_TEMPERATURE) 
                          // || (tempSensors.internal.temp > OVERHEAT_TEMPERATURE);
  if (criticalTemp) {
    switchOn(sw_fan_TRIACs);
    switchOn(sw_fan_Main);
    emergencyShutdown(3, true);
  }

  float delta = DELTA_TEMP;

  float minTemp = tempSensors.external.temp < (MIN_TRIAC_FANS_TEMPERATURE - delta) ? MIN_TRIAC_FANS_TEMPERATURE : tempSensors.external.temp + delta;
  bool turnTRIACsFansOff = (tempSensors.triac1.temp < minTemp);// && (tempSensors.triac2.temp < minTemp) && (tempSensors.triac3.temp < minTemp);
  float maxTemp = tempSensors.external.temp < (MAX_TRIAC_FANS_TEMPERATURE - delta) ? MAX_TRIAC_FANS_TEMPERATURE : tempSensors.external.temp + delta;
  bool turnTRIACsFansOn = (tempSensors.triac1.temp > maxTemp);// || (tempSensors.triac2.temp > maxTemp) || (tempSensors.triac3.temp > maxTemp);
  if (turnTRIACsFansOff) switchOff(sw_fan_TRIACs);
  if (turnTRIACsFansOn) switchOn(sw_fan_TRIACs);

  if (false) { // FIXME Enable
  bool turnMainFanOff = (tempSensors.internal.temp < (tempSensors.external.temp + delta));
  bool turnMainFanOn = (tempSensors.internal.temp > (tempSensors.external.temp + delta + delta));
  if (turnMainFanOff) switchOff(sw_fan_Main);
  if (turnMainFanOn) switchOn(sw_fan_Main);
  }
}


void handleRS485DataRefreshed() {
  dataReceivedTimeMark = millis();

  if (false) {
    Tests::testDataRefresh(sw_InfoPanel_Buzzer, sw_Relay1_ALARM, sw_Relay2_HEAT_FAN, sw_Relay3_POWER, sw_Relay4);
    return;
  }
  
  if (rs485Input.fireAlarm) {
    emergencyShutdown(5, true);
  }
  
  if (rs485InputStored.powerOnRequest != rs485Input.powerOnRequest) {
    rs485InputStored.powerOnRequest = rs485Input.powerOnRequest;
    if (rs485Input.powerOnRequest) {
      powerSystemOn();
    } else {
      powerSystemOff();
    }
  }

  if (rs485InputStored.fanOnRequest != rs485Input.fanOnRequest) {
    rs485InputStored.fanOnRequest = rs485Input.fanOnRequest;
    switchToggleTo(sw_Relay2_HEAT_FAN, rs485Input.fanOnRequest);
  }

  if (rs485Input.heatRequest) { // TODO This runs constantly - make it run once when request changes
    if (!isSystemPoweredOn) return; // do not let Heat On if not Powered On
    if (millis() - systemPowerOnTimeMark < 2000) return; // prevent Heat On before Contactor fully turns on!
    switchOn(sw_Heater_TRIACs);
  } else {
    switchOff(sw_Heater_TRIACs);
  }
}
