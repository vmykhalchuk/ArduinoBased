#include <Arduino.h>
#include "switch_pin.h"
#include "rs485_server.h"
#include "info_panel.h"
#include "ds18b20.h"
#include "tests.h"

const int pin_RS485_dir = 2; // LOW - Listening, HIGH - Transmitting

// 4-6 - TRIACs
// 7 - Internal (inside enclosure)
// 8 - External
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

float _temperatures[TEMP_SENS_COUNT];

bool _isTestMode = false;

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
  dataReceivedTimerMark = millis();
}

void initTempSensors() {
  // init temp sensors
  bool allSensorsPresent = true;
  for (int i = 0; i < 1/*FIXME TEMP_SENS_COUNT*/; i++) {
    allSensorsPresent &= DS18B20::setResolution(pins_DS18B20[i], 0x3F);
    if (!allSensorsPresent) break;
    
    _temperatures[i] = DS18B20::readTemperature(pins_DS18B20[i]);
  }

  if (!allSensorsPresent) {
    // critical error, we cannot continue operation
    fullStopBecauseOfCriticalError(3);
  }
}

void powerSystemOn() {
  if (isSystemPoweredOn) return;
  if (fireAlarm) return; // for safety reasons we do not let system on!

  if (isSwitchOn(sw_Relay3_POWER)) {
    // FIXME log error!
    if (isSwitchOn(sw_Heater_TRIACs)) {
      _delay(1000);
    }
  }
  switchOff(sw_Relay3_POWER);
  
  // toggle Alarm for 3 sec to test it works!
  switchOn(sw_Relay1_ALARM);
  _delay(3000);
  switchOff(sw_Relay1_ALARM);
  switchOn(sw_Relay3_POWER);

  isSystemPoweredOn = true;
  systemPowerOnTimerMark = millis();
}

void powerSystemOff() {
  switchOff(sw_Heater_TRIACs);
  _delay(2000); // we must keep delay between TRIACs and Contactor's transitioning
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

void _delay(uint16_t timeMs) {
  uint8_t intervalMs = 50;
  while (timeMs > 0) {
    uint8_t delMs = timeMs % intervalMs;
    timeMs -= delMs;
    delay(delMs);
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

  if ((millis() - dataReceivedTimerMark) > 10000) {
    if (isSystemPoweredOn) {
      // remote board is unavailable while System Power is ON - we must turn it OFF for safety!!!
      powerSystemOff();
    }
    InfoPanel::setCommunicationError();
    blink(sw_InfoPanel_Buzzer, 3, 50);
    dataReceivedTimerMark = millis();
  }
}


uint8_t _tempSensorsCount = 0;
void loopTempSensors() {
  if (_tempSensorsCount % 40 == 39) {
    uint8_t sensNo = _tempSensorsCount / 40;
    _temperatures[sensNo] = DS18B20::readTemperature(pins_DS18B20[sensNo]);
    handleTempsUpdated();
  }
  _tempSensorsCount++;
  if (_tempSensorsCount / 40 == 1/*FIXME TEMP_SENS_COUNT*/) _tempSensorsCount = 0;
}

void handleTempsUpdated() {
  bool turnTRIACsFansOff = (_temperatures[0] < 29);// && (_temperatures[1] < 23) && (_temperatures[3] < 23);
  bool turnTRIACsFansOn = (_temperatures[0] > 33);// || (_temperatures[1] > 25) || (_temperatures[2] > 25);
  if (turnTRIACsFansOff) switchOff(sw_fan_TRIACs);
  if (turnTRIACsFansOn) switchOn(sw_fan_TRIACs);

  if (false) {
  bool turnMainFanOff = (_temperatures[3] < (_temperatures[4] + 1));
  bool turnMainFanOn = (_temperatures[3] > (_temperatures[4] + 3));
  if (turnMainFanOff) switchOff(sw_fan_Main);
  if (turnMainFanOn) switchOn(sw_fan_Main);
  }
}


void handleRS485DataRefreshed() {
  dataReceivedTimerMark = millis();

  if (false) {
    Tests::testDataRefresh(sw_InfoPanel_Buzzer, sw_Relay1_ALARM, sw_Relay2_HEAT_FAN, sw_Relay3_POWER, sw_Relay4);
    return;
  }
  
  if (RS485Server::f4) fireAlarm = true; // will not reset unless full system resets after power cycling!
  
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

  handleRS485HeatRequestFlag();
}

void handleRS485HeatRequestFlag() {
  if (heatRequest != RS485Server::f3) {
    if (RS485Server::f3) {
      if (!isSystemPoweredOn) return; // do not let Heat On if not Powered On
      if (millis() - systemPowerOnTimerMark < 2000) return; // prevent Heat On before Contactor fully turns on!
    }
    heatRequest = RS485Server::f3;
    switchToggleTo(sw_Heater_TRIACs, heatRequest);
  }
}
