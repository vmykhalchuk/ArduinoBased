const uint8_t pinSwitch = 3; // with pull-up enabled
// pwm is available ony on: 3,5,6,9,10,11
const uint8_t pinLed1 = 5;
const uint8_t pinLed2 = 6;

void setup() {
  Serial.begin(115200);
  pinMode(pinSwitch, INPUT);
  digitalWrite(pinSwitch, HIGH);
  pinMode(pinLed1, OUTPUT);
  pinMode(pinLed2, OUTPUT);
  Serial.println("Started!");
}

/*
// switch debouncing part
const uint16_t settleTime = 50;
bool switchState = HIGH;
bool switchStateSettling = false;
uint16_t firstStateChangeMs = 0;

void loop() {
  bool _s = digitalRead(pinSwitch);
  uint16_t _m = millis();
  
  if (!switchStateSettling && _s != switchState) {
    firstStateChangeMs = _m;
    switchStateSettling = true;
  }
  
  if (switchStateSettling && (_m - firstStateChangeMs) > settleTime) {
    switchStateSettling = false;
    switchState = _s;
  }
}
*/

const uint8_t minPwmState = 20;
bool ledStateIsOn = false;
uint8_t ledPwmState = 230;
bool ascendLedPwm = false;

// switch debouncing part
const uint16_t settleTime = 50;
bool switchState = HIGH;

// switch click/longclick part
const uint16_t shortClickMaxTimeMs = 500;
const uint16_t longClickMinTimeMs = 1000;
const uint16_t longClickStepMs = 10;
bool clickStarted = false;
uint16_t clickStartMs = 0;
uint8_t initialLedPwmState = 0;

void fadeOut(uint8_t pin, uint8_t currentValue) {
  for (uint8_t i = currentValue; i>=1; i--) {
    delay(3);
    analogWrite(pin,i);
  }
  delay(3);
  analogWrite(pin,0);
}

void loop() {
  bool switchStateChanged = false;
  if (digitalRead(pinSwitch) != switchState) {
    delay(settleTime);
    bool newSwitchState = digitalRead(pinSwitch);;
    if (switchState != newSwitchState) {
      switchStateChanged = true;
    }
    switchState = newSwitchState;
  }

  bool _clickFinished = false;
  uint16_t _ms = millis();
  if (switchStateChanged) {
    if (switchState == LOW) {
      clickStarted = true;
      clickStartMs = _ms;
    } else {
      clickStarted = false;
      _clickFinished = true;
    }
  }

  bool _ledStateToggle = false;
  if (_clickFinished) {
    if ((_ms - clickStartMs) < shortClickMaxTimeMs) {
      // short click (toggle switch)
      _ledStateToggle = true;

      ledStateIsOn = !ledStateIsOn;
      if (ledStateIsOn) {
        analogWrite(pinLed1, ledPwmState);
        analogWrite(pinLed2, ledPwmState);
      } else {
        fadeOut(pinLed1, ledPwmState);//analogWrite(pinLed1, 0);
        fadeOut(pinLed2, ledPwmState);//analogWrite(pinLed2, 0);
      }
    } else if ((_ms - clickStartMs) >= longClickMinTimeMs) {
      //Serial.println();
      //Serial.println("TOGGLED PWM ASCENDING!");
      ascendLedPwm = !ascendLedPwm;
    }
  
  } else if (clickStarted && (_ms - clickStartMs) >= longClickMinTimeMs) {

    uint16_t steps = (_ms - clickStartMs - longClickMinTimeMs) / longClickStepMs;
    if (steps == 0) {
      initialLedPwmState = ledPwmState;
    } else if (steps < 300) {
      uint16_t r = 10000 + initialLedPwmState - minPwmState;
      if (ascendLedPwm) r += steps;
      else r -= steps;

      uint8_t rr;
      if (r < 10000) rr = 0;
      else if (r > (10255-minPwmState)) rr = 255-minPwmState;
      else rr = (r - 10000);

      //Serial.print(rr,HEX); Serial.print(" ");
      ledPwmState = rr + minPwmState;
      analogWrite(pinLed1, ledPwmState);
      analogWrite(pinLed2, ledPwmState);
      ledStateIsOn = true;
    }
  }
}
