/*

  This controller will monitor switch inputs and filter all contacts jitter or signal noise.
  Outputs will be commanded ON/OFF as a result of input analysis.
  
  A0 => D9
  A1 => D8
  A2 => D7
  A3 => D6
  A4 => D5
  A5 => D4
*/

/*

  Tech info:
    - https://www.instructables.com/Arduino-and-Port-Manipulation/
    - A0-A5 correeespond to PC0-PC5
*/

const uint8_t MAP_COUNT = 6;
const uint8_t pinOutputs[MAP_COUNT] = {4,5,6,7,8,9};

uint8_t states = 0;
uint8_t inputChangeRequested = 0;
unsigned long inputChangeLastAttemptTimeMs[MAP_COUNT];

void setup() {
  // https://www.instructables.com/Arduino-and-Port-Manipulation/
  DDRC = DDRC & B11000000; // set PORTC (digital 5~0) to inputs
  PORTC = PORTC | B00111111; // enable PORTC (digital 5~0) pull-ups

  for (uint8_t i = 0; i < MAP_COUNT; i++) {
    inputChangeLastAttemptTimeMs[i] = 0;
    pinMode(pinOutputs[i], OUTPUT);
  }
}

void loop() {
  loop_test();
}

void loop_test() {
  uint8_t d = PINC & B00111111;
  if (states != d) {
    states = d;
    for (uint8_t i = 0; i < MAP_COUNT; i++) {
      uint8_t mask = 2 << i;
      digitalWrite(pinOutputs[i], (states & mask)? HIGH : LOW);
    }
  }
}

uint8_t previousData = 0;
void loop_main() {
  uint8_t data = PINC & B00111111;
  if (data != previousData) {
    handleNewData(data, data ^ previousData);
    previousData = data;
  }
}

void handleNewData(uint8_t data, uint8_t changesMask) {
  if (states != data) {
    for (uint8_t i = 0; i < MAP_COUNT; i++) {
      uint8_t mask = 2 << i;
      if (states & mask != data & mask) {
        if (!(inputChangeRequested & mask)) {
          inputChangeRequested = inputChangeRequested | mask;
          inputChangeLastAttemptTimeMs[i] = millis();
        }
      }
    }
  }

  if (inputChangeRequested) {
    for (uint8_t i = 0; i < MAP_COUNT; i++) {
      uint8_t mask = 2 << i;
      if (inputChangeRequested & mask) {
        if ((inputChangeLastAttemptTimeMs[i] - millis()) > 150) {
          if (states & mask != data & mask) {
            // commit change
          }
        }
      }
    }
  }

        /*if ((inputChangeLastAttemptTimeMs[i] - millis()) > 150) {
          // commit state change
          states = states & !mask;
          states = states | (d & mask);
          digitalWrite(pinOutputs[i], (states & mask) ? HIGH : LOW);
          inputChangeRequested = inputChangeRequested & !mask;
        } else {
          // reset counter
          inputChangeLastAttemptTimeMs[i] = millis();
        }*/
}
