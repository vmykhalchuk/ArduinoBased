#include "avr/pgmspace.h"
#include "avr/io.h"

#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

void setup() {
  Serial.begin(9600);
  cbi (TIMSK0, TOIE0); // disable Timer0 !!! delay() is now not available
  cli(); // disable interrupts globally
  sei(); // enable interrupts globally
  asm volatile (
    // DDRC = 0010 0000, ie. PORTB5 is OUT
    "ldi r16,48 \n"
    "out 0x27,r16 \n"
  );

  asm volatile (
    "cli \n"
    " \n"
    " \n"
    " \n"
    " \n"
    " \n"
    " \n"
    " \n"
    " \n"
    " \n"
    " \n"
    " \n"
    " \n"
    " \n"
    " \n"
  );
}

void loop() {
}
