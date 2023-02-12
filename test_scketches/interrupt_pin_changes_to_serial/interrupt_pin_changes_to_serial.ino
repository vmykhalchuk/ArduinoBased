const uint8_t pinInput = 3; // D2 or D3 support interrupt on Arduino Nano/Uno (ATMEGA328)

const uint8_t pinInputMask = 1 << pinInput;

volatile bool inputPinState;
volatile uint16_t interruptRoutineMicros;
uint16_t results[256];
uint8_t savedInputPinSates[32]; // 32 * 8 = 256 states
volatile uint8_t resultsHeadPos = 0;
volatile uint8_t resultsTailPos = 0;

volatile uint16_t error1 = 0;
volatile uint16_t error2 = 0;

volatile uint16_t _micros;

void setup() {
  pinMode(pinInput, INPUT); digitalWrite(pinInput, HIGH);

  Serial.begin(9600);
  while(!Serial) {
    // some boards need to wait to ensure access to serial over USB
  }

  Serial.println("Interrupt on pin tester v0.1 (2023Jan09)");

  _micros = micros();
  interruptRoutineMicros = _micros;
  inputPinState = PIND & pinInputMask;//digitalRead(pinInput);
  attachInterrupt(digitalPinToInterrupt(pinInput), pinInputChangeInterruptRoutine, CHANGE);
}

const uint8_t andMasks[] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
const uint8_t minusMasks[] = { 0xFF-0x01, 0xFF-0x02, 0xFF-0x04, 0xFF-0x08, 0xFF-0x10, 0xFF-0x20, 0xFF-0x40, 0xFF-0x80 };

void pinInputChangeInterruptRoutine() {
  bool newInputPinState = digitalRead(pinInput); //PIND & pinInputMask; //digitalRead(pinInput);
  if (inputPinState == newInputPinState) {
    // error, missed change
    if (error1 != 0xFFFF) error1++;
  }
    uint16_t res = micros() - interruptRoutineMicros;
    interruptRoutineMicros += res;
  inputPinState = newInputPinState;

  uint8_t newHeadPos = resultsHeadPos + 1;
  if (resultsTailPos == newHeadPos) {
    // error 2 - overflow!!!
    // so we skip newer results till we get space
    if (error2 != 0xFFFF) error2++;
  } else {
    results[resultsHeadPos] = res;
    if (newInputPinState) {
      savedInputPinSates[resultsHeadPos >> 3] |= andMasks[resultsHeadPos & 7];
    } else {
      savedInputPinSates[resultsHeadPos >> 3] &= minusMasks[resultsHeadPos & 7];
    }
    resultsHeadPos = newHeadPos;
  }
}

void loop() {
  //_micros = micros();
  if (resultsHeadPos != resultsTailPos) {
    if (savedInputPinSates[resultsTailPos >> 3] & andMasks[resultsTailPos & 7]) {
      Serial.print('H');
    } else {
      Serial.print('L');
    }
    Serial.println(results[resultsTailPos]);
    resultsTailPos++;
  }
  if (error1 > 0) {
    uint16_t e = error1; error1 = error1 - e;
    Serial.print("E1:"); if (e == 0xFFFF) Serial.println("OVRFLW"); else Serial.println(e);
  }
  if (error2 > 0) {
    uint16_t e = error2; error2 = error2 - e;
    Serial.print("E2:"); if (e == 0xFFFF) Serial.println("OVRFLW"); else Serial.println(e);
  }
}