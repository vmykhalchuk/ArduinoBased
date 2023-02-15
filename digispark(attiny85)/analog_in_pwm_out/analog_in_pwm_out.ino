// http://digistump.com/wiki/digispark/tutorials/basics#analog_writeaka_pwm
int pinPwmOut = 0; // P0 (0,1,4)
// http://digistump.com/wiki/digispark/tutorials/basics#analog_read
int pinAnalogIn = 1; //P2 (0->P5; 1->P2; 2->P4; 3->P3)

#include "DigiKeyboard.h"

void setup() {
  pinMode(pinPwmOut, OUTPUT);
}

// measure time of analogRead (108..224)
void loop_2() {
  uint32_t _min=-1,_max=0;

  for (int i = 0; i < 10000; i++) {
    uint32_t _m = micros();
    int v = analogRead(pinAnalogIn);
    //for (int j = 0; j < 1; j++) __asm__("nop\n\t");
    uint32_t r = micros() - _m;
    _min = min(_min, r);
    _max = max(_max, r);
  }

  DigiKeyboard.sendKeyStroke(0);
  DigiKeyboard.print("Min: "); DigiKeyboard.print(_min);
  DigiKeyboard.print("\tMax: "); DigiKeyboard.print(_max);
  DigiKeyboard.println();
}

int i = 0;
void loop() {
  int v = analogRead(pinAnalogIn);
  int r = map(v, 0, 1023, 0, 255);
  analogWrite(pinPwmOut, r);

  if (i++ >= 10000) {
    i = 0;
    DigiKeyboard.sendKeyStroke(0);
    DigiKeyboard.print("V: "); DigiKeyboard.println(v);
  }
}