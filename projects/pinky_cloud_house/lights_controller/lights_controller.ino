/*
  This controller will monitor switch inputs and filter all contacts jitter or signal noise.
  Outputs will be commanded ON/OFF as a result of input analysis.
  If input is unstable - error pin is raised HIGH.

  INPUT  => OUTPUT  => ERROR
 -------------------------
 A0(PC0) => D2(PD2) =>  D8(PB0)
 A1(PC1) => D3(PD3) =>  D9(PB1)
 A2(PC2) => D4(PD4) =>  D10(PB2)
 A3(PC3) => D5(PD5) =>  D11(PB3)
 A4(PC4) => D6(PD6) =>  D12(PB4)
 A5(PC5) => D7(PD7) =>  D13(PB5)
*/

/*
  Tech info:
    - https://www.instructables.com/Arduino-and-Port-Manipulation/
    - A0-A5 correeespond to PC0-PC5
*/

const uint8_t MAP_COUNT = 6;
uint8_t outputPinMaskArr[MAP_COUNT];
uint8_t errorPinMaskArr[MAP_COUNT];
uint8_t invOutputPinMaskArr[MAP_COUNT];
uint8_t invErrorPinMaskArr[MAP_COUNT];

uint8_t states = 0;

// for solution #1
uint8_t inputChangeRequested = 0;
unsigned long inputChangeLastAttemptTimeMs[MAP_COUNT];

void setup() {
  // https://www.instructables.com/Arduino-and-Port-Manipulation/
  DDRC  = DDRC &  B11000000; // set PORTC(5-0) as inputs
  PORTC = PORTC | B00111111; // enable PORTC(5-0) pull-ups

  DDRD  = DDRD |  B11111100; // set PORTD(7-2) as outputs
  PORTD = PORTD & B00000011; // set PORTD(7-2) to LOW

  DDRB  = DDRB |  B00111111; // set PORTB(5-0) as outputs
  PORTB = PORTB & B11000000; // set PORTB(5-0) to LOW

  for (uint8_t i = 0; i < MAP_COUNT; i++) {
    inputChangeLastAttemptTimeMs[i] = 0;
    errorPinMaskArr[i] = 1 << i;
    invErrorPinMaskArr[i] = !errorPinMaskArr[i];
    outputPinMaskArr[i] = errorPinMaskArr[i] << 2;
    invOutputPinMaskArr[i] = outputPinMaskArr[i];
  }

  // test outputs
  for (uint8_t i = 0; i < MAP_COUNT; i++) {
    PORTD = PORTD | outputPinMaskArr[i];
    PORTB = PORTB | errorPinMaskArr[i];
    delay(100);
    PORTD = PORTD & invOutputPinMaskArr[i];
    PORTB = PORTB & invErrorPinMaskArr[i];
    delay(200);
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
      uint8_t mask = 1 << i;
      if (states & mask) {
        PORTD = PORTD | outputPinMaskArr[i];
      } else {
        PORTD = PORTD & invOutputPinMaskArr[i];
      }
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
  const uint8_t lowLevelThresholdPercentage = 30; // below this level is considered 0
  const uint8_t highLevelThresholdPercentage = 70; // above this level is considered 1
  const uint16_t t1 = ((uint16_t) statisticsSize * 100) / lowLevelThresholdPercentage;
  const uint8_t lowLevelThresholdCount = t1;
  const uint16_t t2 = ((uint16_t) statisticsSize * 100) / highLevelThresholdPercentage;
  const uint8_t highLevelThresholdCount = t2;

  uint8_t masks[8]; uint8_t invMasks[8]; for (uint8_t i = 0; i < 8; i++) { masks[i] = 2 << i; invMasks[i] = (!masks[i]); }
  
  while (true) {
    uint8_t data = PINC & B00111111;

    // update statistics & statisticsHighLevelCount with new observation (data)
    for (uint8_t i = 0; i < MAP_COUNT; i++) {
      uint8_t mask = 1 << i;
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
      uint8_t newPinState = 0;
      if (hlc >= highLevelThresholdCount) {
        newPinState = mask;
        PORTB = PORTB & invErrorPinMaskArr[i]; // clear error pin[i]
      } else if (hlc <= lowLevelThresholdCount) {
        newPinState = 0;
        PORTB = PORTB & invErrorPinMaskArr[i]; // clear error pin[i]
      } else {
        PORTB = PORTB | errorPinMaskArr[i]; // set error pin[i]
        continue;
      }

      uint8_t oldPinState = states & mask;
      uint16_t t = 0; // inertiaTicks // FIXME for testing we have it zero, when tested - change it back to inertiaTicks
      if (newPinState != oldPinState && outputLastTimeUpdatedTicksAgo[i] >= t) {
        if (newPinState == 0) {
          PORTD = PORTD & invOutputPinMaskArr[i]; // clear out pin[i]
          states = states & invMask;
        } else {
          PORTD = PORTD | outputPinMaskArr[i]; // set out pin[i]
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
      uint8_t mask = 1 << i;
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
      uint8_t mask = 1 << i;
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
