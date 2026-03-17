#include <Arduino.h>
#include "rs485_client.h"

int pin_RS485_dir = 2; // when LOW - Transmitting, HIGH - Receiving

// 3 data bytes + crc
uint8_t rx485_in_buf[1];
uint8_t rx485_out_buf[3];

bool powerOnRequest = false;
bool fanOnRequest = false;
bool heatRequest = false;
bool fireAlarm = false; // in case fire is registered!

void setup() {
  RS485Client::initRS485Client(pin_RS485_dir);
  Serial.begin(9600);
}

int i = 0;
void loop() {
  i++;
  if (i%2 == 0) RS485Client::rs485ClientUpdateFlags(true,false,true,false); else RS485Client::rs485ClientUpdateFlags(false,true,false, true);
  RS485Client::rs485ClientLoop();
}

uint8_t state_tempSensors = 0;

void loop_tempSensors() {
}

void loop_buttons() {
  
}

void loop_digitsDisplay() {
}

void loop_leds() {
}
