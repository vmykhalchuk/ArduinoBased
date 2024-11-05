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

// for solution #1
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

void loop_main_solution2() {
  uint8_t error = 0;

  const uint8_t statisticsSize = 100; // by changing this value we modify system reactio time (reducing value increases reaction time and vice versa)
  uint8_t statisticsPointer = 0; // where pointer is currently waiting for data to come in
  uint8_t statistics[statisticsSize]; for (uint8_t i = 0; i < statisticsSize; i++) statistics[i] = 0;
  uint8_t statisticsHighLevelCount[MAP_COUNT]; for (uint8_t i = 0; i < MAP_COUNT; i++) statisticsHighLevelCount[i] = 0;
  
  const uint16_t inertiaTicks = 1000;//for how many ticks output state will not change to new value
  uint16_t outputLastTimeUpdatedTicksAgo[MAP_COUNT]; for (uint8_t i = 0; i < MAP_COUNT; i++) outputLastTimeUpdatedTicksAgo[i] = inertiaTicks;
  const uint8_t highLevelThresholdPercentage = 70;
  const uint16_t t = ((uint16_t) statisticsSize * 100) / highLevelThresholdPercentage;
  const uint8_t highLevelThresholdCount = t;

  uint8_t masks[8]; uint8_t invMasks[8]; for (uint8_t i = 0; i < 8; i++) { masks[i] = 2 << i; invMasks[i] = (!masks[i]); }
  
  while (true) {
    uint8_t data = PINC & B00111111;

    // update statistics & statisticsHighLevelCount with new observation (data)
    for (uint8_t i = 0; i < MAP_COUNT; i++) {
      uint8_t mask = 2 << i;
      if (statistics[statisticsPointer] & mask != 0) {
        if (statisticsHighLevelCount[i] == 0) error = 0x01;
        else statisticsHighLevelCount[i]--;
      }
      if (data & mask != 0) {
        if (statisticsHighLevelCount[i] > statisticsSize) error = 0x02;
        else statisticsHighLevelCount[i]++;
      }
    }
    statistics[statisticsPointer] = data;

    // increase tick counters for every value changed
    for (uint8_t i = 0; i < MAP_COUNT; i++) if (outputLastTimeUpdatedTicksAgo[i] < inertiaTicks) outputLastTimeUpdatedTicksAgo[i]++;
    
    // update outputs to represent new stats
    for (uint8_t i = 0; i < MAP_COUNT; i++) {
      uint8_t mask = masks[i];
      uint8_t invMask = invMasks[i];
      uint8_t hlc = statisticsHighLevelCount[i];
      uint8_t newPinState = hlc >= highLevelThresholdCount ? mask : 0;
      uint8_t oldPinState = states & mask;
      uint16_t t = 0; // inertiaTicks // FIXME for testing we have it zero, when tested - change it back
      if (newPinState != oldPinState && outputLastTimeUpdatedTicksAgo[i] >= t) {
        if (newPinState == 0) {
          digitalWrite(pinOutputs[i], LOW);
          states = states & invMask;
        } else {
          digitalWrite(pinOutputs[i], HIGH);
          states = states | mask;
        }
        outputLastTimeUpdatedTicksAgo[i] = 0;
      }
    }
  }
}

uint8_t previousData = 0;
void loop_main_solution1() {
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
