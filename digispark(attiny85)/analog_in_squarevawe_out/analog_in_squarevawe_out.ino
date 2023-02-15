// http://digistump.com/wiki/digispark/tutorials/basics#analog_read
int pinAnalogIn = 1; //P2 (0->P5; 1->P2; 2->P4; 3->P3)

#include <avr/io.h>
#define OUT_PIN PB3

void setup() {
  DDRB |= (1 << OUT_PIN);
}

void loop() {
  // https://www.instructables.com/ATTiny-Port-Manipulation/
  // PORTB |= (1 << OUT_PIN); // set
  // PORTB &= ~(1 << OUT_PIN); // clear

  int z = analogRead(pinAnalogIn);

  //int v = 5000 + z; // 10 ... 10_000
  int v = map(z, 0, 1023, 10, 10000);
  int cycles = 50000 / v;

  for (int j = 0; j < cycles; j++) {
    PORTB ^= (1 << OUT_PIN);
    for (int i = 0; i < v; i++) __asm__("nop\n\t");
  }
}
