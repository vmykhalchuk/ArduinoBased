#include <OneWire.h>
#include <DallasTemperature.h>

// Sensor Data Wire (externally pulled-up with 4.7k)
#define ONE_WIRE_BUS 4

OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  sensors.begin();

  for (int i = 0; i < 20; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    setDisplayBufToErrorMsg();
    for (uint8_t i = 0; i < 5; i++) displayLoop25ms();
    digitalWrite(LED_BUILTIN, LOW);
    clearDisplayBuf();
    for (uint8_t i = 0; i < 5; i++) displayLoop25ms();
  }
}

uint8_t displayBuf[] = {0,0,0};

void loop() {
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);
  bool alarm = tempC > 35;
  
  // Check if reading was successful
  if (tempC != DEVICE_DISCONNECTED_C)
  {
    setDisplayBufToFloatWithOneDecimal(tempC);
  }
  else
  {
    Serial.println("Error: Could not read temperature data");
  }

  // display loop
  for (int outerI = 0; outerI < 4; outerI++) {
    if (alarm) {
      if (outerI%2 == 0) digitalWrite(LED_BUILTIN, HIGH);
      else digitalWrite(LED_BUILTIN, LOW);
    }
    for (uint8_t i = 0; i < 50; i++) displayLoop25ms();
  }
  clearDAll();
}


// Display input->Arduino port: 1->D5;2->D6;3->D7;4->B0;5->B1;6->B2

// takes around 25ms to execute
void displayLoop25ms() {
  for (uint8_t j = 0; j < 3; j++) {
    uint8_t b = displayBuf[j];
    for (uint8_t k = 0; k < 8; k++) {
      clearDAll();
      uint8_t m = 1 << k;
      if (b & m) lightDSegment(j * 8 + k);
      delay(1);
    }
  }
}

void lightDSegment(uint8_t s) {
  if (s == 0) {
    setDPinLow(3); setDPinHigh(4);
  } else if (s == 1) {
    setDPinLow(3); setDPinHigh(5);
  } else if (s == 2) {
    setDPinLow(1); setDPinHigh(2);
  } else if (s == 3) {
    setDPinLow(1); setDPinHigh(3);
  } else if (s == 4) {
    setDPinLow(1); setDPinHigh(4);
  } else if (s == 5) {
    setDPinLow(1); setDPinHigh(5);
  } else if (s == 6) {
    setDPinLow(1); setDPinHigh(6);
  } else if (s == 7) {
    setDPinLow(2); setDPinHigh(3);
  } else if (s == 8) {
    setDPinLow(2); setDPinHigh(4);
  } else if (s == 9) {
    setDPinLow(2); setDPinHigh(1);
  } else if (s == 10) {
    setDPinLow(3); setDPinHigh(1);
  } else if (s == 11) {
    setDPinLow(4); setDPinHigh(1);
  } else if (s == 12) {
    setDPinLow(5); setDPinHigh(1);
  } else if (s == 13) {
    setDPinLow(6); setDPinHigh(1);
  } else if (s == 14) {
    setDPinLow(2); setDPinHigh(5);
  } else if (s == 15) {
    setDPinLow(2); setDPinHigh(6);
  } else if (s == 16) {
    setDPinLow(3); setDPinHigh(6);
  } else if (s == 17) {
    setDPinLow(3); setDPinHigh(2);
  } else if (s == 18) {
    setDPinLow(4); setDPinHigh(2);
  } else if (s == 19) {
    setDPinLow(5); setDPinHigh(2);
  } else if (s == 20) {
    setDPinLow(6); setDPinHigh(2);
  } else if (s == 21) {
    setDPinLow(4); setDPinHigh(3);
  } else if (s == 22) {
    setDPinLow(5); setDPinHigh(3);
  } else if (s == 23) {
    setDPinLow(6); setDPinHigh(3);
  }
}

void clearDAll() {
  DDRD = DDRD & B00011111;
  PORTD = PORTD & B00011111;
  DDRB = DDRB & B11111000;
  PORTB = PORTB & B11111000;
}

void setDPinHigh(uint8_t p) {
  if (p == 1) {
    DDRD = DDRD | B00100000;
    PORTD = PORTD | B00100000;
  } else if (p == 2) {
    DDRD = DDRD | B01000000;
    PORTD = PORTD | B01000000;
  } else if (p == 3) {
    DDRD = DDRD | B10000000;
    PORTD = PORTD | B10000000;
  } else if (p == 4) {
    DDRB = DDRB | B00000001;
    PORTB = PORTB | B00000001;
  } else if (p == 5) {
    DDRB = DDRB | B00000010;
    PORTB = PORTB | B00000010;
  } else if (p == 6) {
    DDRB = DDRB | B00000100;
    PORTB = PORTB | B00000100;
  }
}

void setDPinLow(uint8_t p) {
  if (p == 1) {
    DDRD = DDRD | B00100000;
    PORTD = PORTD & B11011111;
  } else if (p == 2) {
    DDRD = DDRD | B01000000;
    PORTD = PORTD & B10111111;
  } else if (p == 3) {
    DDRD = DDRD | B10000000;
    PORTD = PORTD & B01111111;
  } else if (p == 4) {
    DDRB = DDRB | B00000001;
    PORTB = PORTB & B11111110;
  } else if (p == 5) {
    DDRB = DDRB | B00000010;
    PORTB = PORTB & B11111101;
  } else if (p == 6) {
    DDRB = DDRB | B00000100;
    PORTB = PORTB & B11111011;
  }
}

void setDisplayBufToFloatWithOneDecimal(float f) {
  int d100 = (int)(f / 100) % 10;
  int d10 = (int)(f / 10) % 10;
  int d1 = (int)f % 10;
  int d0 = (int)(f * 10) % 10;
  if (d100 < 2) {
    setDisplayBuf(d100,d10,d1,d0,true);
  } else {
    setDisplayBuf(0,9,9,9,false);
  }
}

void setDisplayBuf(uint8_t dig1, uint8_t dig2, uint8_t dig3, uint8_t dig4, bool point) {
  clearDisplayBuf();
  
  updateDisplayBufDigitN(4, dig4);
  if (dig3 != 0 || point || dig1 != 0 || dig2 != 0) {
    updateDisplayBufDigitN(3, dig3);
  }
  if ((dig2 != 0) || (dig1 != 0)) {
    updateDisplayBufDigitN(2, dig2);
  }
  if (dig1 == 1) {
    displayBuf[0] = displayBuf[0] | B0000011;
  }
  if (point) {
    displayBuf[2] = displayBuf[2] | B0000001;
  }
}

// Err will be displayed
void setDisplayBufToErrorMsg() {
  clearDisplayBuf();

  // segments to light: E: 2,5,6,7,8; r: 13,15; r: 21,23
  displayBuf[0] = B11100100;
  displayBuf[1] = B10100001;
  displayBuf[2] = B10100000;
}

void clearDisplayBuf() {
  displayBuf[0] = 0; displayBuf[1] = 0; displayBuf[2] = 0;
}

// digitNo: [1;4]
// value:[0;9]; if digitNo==1 value: [0;1]
void updateDisplayBufDigitN(uint8_t digitNo, uint8_t v) {
  uint8_t seg = 0;
  if (v == 0) {
    seg = B0111111;
  } else if (v == 1) {
    seg = B0000110;
  } else if (v == 2) {
    seg = B1011011;
  } else if (v == 3) {
    seg = B1001111;
  } else if (v == 4) {
    seg = B1100110;
  } else if (v == 5) {
    seg = B1101101;
  } else if (v == 6) {
    seg = B1111101;
  } else if (v == 7) {
    seg = B0000111;
  } else if (v == 8) {
    seg = B1111111;
  } else if (v == 9) {
    seg = B1101111;
  }

  if (digitNo == 2) {
    displayBuf[0] = displayBuf[0] | seg << 2;
    displayBuf[1] = displayBuf[1] | (seg & B1000000) >> 6;
  } else if (digitNo == 3) {
    displayBuf[1] = displayBuf[1] | seg << 1;
  } else if (digitNo == 4) {
    displayBuf[2] = displayBuf[2] | seg << 1;
  }
}
