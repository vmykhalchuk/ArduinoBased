const int pinBtn1 = 2; // PB2
const int pinLed = 1;  // PB1

#include "DigiKeyboard.h"

void setup() {
  pinMode(pinBtn1, INPUT_PULLUP);
  pinMode(pinLed, OUTPUT);
}

int i = 0;
void loop() {
  delay(200);
  if (digitalRead(pinBtn1)) {
    digitalWrite(pinLed, HIGH);
  } else {
    digitalWrite(pinLed, LOW);
  }

  if (i++ >= 10) {
    DigiKeyboard.sendKeyStroke(KEY_ARROW_LEFT);
    i = 0;
  }
}