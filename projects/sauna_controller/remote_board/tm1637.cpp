#include "tm1637.h"

// Seven-segment patterns for 0-9
static const uint8_t segmentMap[] = {
  0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f
};

static bool isInitialized = false;
static int pinClk;
static int pinDio;

void TM1637::init(int clockPin, int dioPin) {
  pinClk = clockPin;
  pinDio = dioPin;
  pinMode(pinClk, OUTPUT);
  pinMode(pinDio, OUTPUT);
  isInitialized = true;
}

// --- Low Level Communication ---

static void TM1637::start() {
  digitalWrite(pinClk, HIGH);
  digitalWrite(pinDio, HIGH);
  delayMicroseconds(5); // can be 2 (try it first), same for all 5us
  digitalWrite(pinDio, LOW);
}

static void TM1637::stop() {
  digitalWrite(pinClk, LOW);
  digitalWrite(pinDio, LOW);
  delayMicroseconds(5);
  digitalWrite(pinClk, HIGH);
  digitalWrite(pinDio, HIGH);
  delayMicroseconds(5);
}

static bool TM1637::writeByte(uint8_t b) {
  for (uint8_t i = 0; i < 8; i++) {
    digitalWrite(pinClk, LOW);
    digitalWrite(pinDio, (b & 0x01) ? HIGH : LOW); // Send LSB first
    b >>= 1;
    delayMicroseconds(5);
    digitalWrite(pinClk, HIGH);
    delayMicroseconds(5);
  }
  
  // Wait for ACK
  digitalWrite(pinClk, LOW);
  pinMode(pinDio, INPUT);
  delayMicroseconds(5);
  digitalWrite(pinClk, HIGH);
  delayMicroseconds(5);
  bool ack = digitalRead(pinDio) == 0;
  pinMode(pinDio, OUTPUT);
  return ack;
}

// --- High Level Functions ---

void TM1637::updateDisplay(int num, bool showColon) {
  uint8_t digits[4];
  digits[0] = segmentMap[(num / 1000) % 10];
  digits[1] = segmentMap[(num / 100) % 10];
  digits[2] = segmentMap[(num / 10) % 10];
  digits[3] = segmentMap[num % 10];

  // apply colon to second digit
  if (showColon) {
    digits[1] |= 0x80;
  }

  // 1. Data Command: Setting automatic address adding
  start();
  writeByte(0x40); 
  stop();

  // 2. Address Command: Set start address to 0xC0 (first digit)
  start();
  writeByte(0xC0);
  for (int i = 0; i < 4; i++) {
    writeByte(digits[i]);
  }
  stop();

  // 3. Display Control: Turn on display, set brightness (0x88 is on, 0x8f is max)
  start();
  writeByte(0x88 + 3); // 0x88 is "ON", +7 is max brightness 
  stop();
}
