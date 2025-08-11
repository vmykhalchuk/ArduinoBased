#include <OneWire.h>
#include <DallasTemperature.h>

// Sensor Data Wire (externally pulled-up with 4.7k)
#define ONE_WIRE_BUS 4

OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(9600);
  sensors.begin();
}

int _c = 0;
uint8_t s = 0;

uint8_t displayB[] = {0,0,0};

void loop() {
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);
  
  // Check if reading was successful
  if (tempC != DEVICE_DISCONNECTED_C)
  {
    Serial.print("Temperature for the device 1 (index 0) is: ");
    Serial.println(tempC);

    int d100 = (int)(tempC / 100) % 10;
    int d10 = (int)(tempC / 10) % 10;
    int d1 = (int)tempC % 10;
    int d0 = (int)(tempC * 10) % 10;
    Serial.print("[");
    Serial.print(d100);
    Serial.print(",");
    Serial.print(d10);
    Serial.print(",");
    Serial.print(d1);
    Serial.print(",");
    Serial.print(d0);
    Serial.println("]");
  }
  else
  {
    Serial.println("Error: Could not read temperature data");
  }

  displayB[0] = 0; displayB[1] = 0; displayB[2] = 0;
  displayB[s/8] = displayB[s/8] | (1 >> (s%8));

  // display loop
  for (uint8_t i = 0; i < 50; i++) {
    for (uint8_t j = 0; j < 3; j++) {
      uint8_t b = displayB[j];
      for (uint8_t k = 0; k < 8; k++) {
        clearDAll();
        uint8_t m = 1 >> k;
        if (b&m) setDSegment(j * 8 + k);
        delay(1);
      }
    }
  }

  //displayDigits(_c++);
  //delay(3000);
}


// Display input->Arduino port: 1->D5;2->D6;3->D7;4->B0;5->B1;6->B2

void displayDigits(int c) {
  c = c % 6; //12;
  if (c == 0) {
    setDPin(1,LOW); setDPin(2,HIGH);
    clearDPin(3); clearDPin(5);//setDPin(3,LOW); setDPin(5,HIGH);
    clearDPin(4); clearDPin(6);
  } else if (c == 1) {
    setDPin(1,LOW); setDPin(3,HIGH);
    clearDPin(2); clearDPin(6);//setDPin(2,LOW); setDPin(6,HIGH);
    clearDPin(4); clearDPin(5);
  } else if (c == 2) {
    clearDPin(1); clearDPin(2);
    clearDPin(3); clearDPin(4);
    clearDPin(5); clearDPin(6);
    
  } else if (c == 3) {
    setDPin(1,LOW); setDPin(2,HIGH);
    setDPin(3,LOW); setDPin(5,HIGH);
    clearDPin(4); clearDPin(6);
  } else if (c == 4) {
    setDPin(1,LOW); setDPin(3,HIGH);
    setDPin(2,LOW); setDPin(6,HIGH);
    clearDPin(4); clearDPin(5);
  } else if (c == 5) {
    clearDPin(1); clearDPin(2);
    clearDPin(3); clearDPin(4);
    clearDPin(5); clearDPin(6);
  }
}

void setDPin(uint8_t p, uint8_t setHigh) {
  if (p == 1) {
    DDRD = DDRD | B00100000;
    if (setHigh == HIGH) PORTD = PORTD | B00100000; else PORTD = PORTD & B11011111;
  } else if (p == 2) {
    DDRD = DDRD | B01000000;
    if (setHigh == HIGH) PORTD = PORTD | B01000000; else PORTD = PORTD & B10111111;
  } else if (p == 3) {
    DDRD = DDRD | B10000000;
    if (setHigh == HIGH) PORTD = PORTD | B10000000; else PORTD = PORTD & B01111111;
  } else if (p == 4) {
    DDRB = DDRB | B00000001;
    if (setHigh == HIGH) PORTB = PORTB | B00000001; else PORTB = PORTB & B11111110;
  } else if (p == 5) {
    DDRB = DDRB | B00000010;
    if (setHigh == HIGH) PORTB = PORTB | B00000010; else PORTB = PORTB & B11111101;
  } else if (p == 6) {
    DDRB = DDRB | B00000100;
    if (setHigh == HIGH) PORTB = PORTB | B00000100; else PORTB = PORTB & B11111011;
  }
}
void clearDPin(uint8_t p) {
  if (p == 1) {
    DDRD = DDRD & B11011111;
  } else if (p == 2) {
    DDRD = DDRD & B10111111;
  } else if (p == 3) {
    DDRD = DDRD & B01111111;
  } else if (p == 4) {
    DDRB = DDRB & B11111110;
  } else if (p == 5) {
    DDRB = DDRB & B11111101;
  } else if (p == 6) {
    DDRB = DDRB & B11111011;
  }
}

void setDSegment(uint8_t s) {
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
