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
  
  
  displayDigits(_c++);

  for (int i = 0; i < 50; i++) {
    displayDigits(0);
    delay(1);
    displayDigits(1);
    delay(1);
    displayDigits(3);
    delay(1);
    displayDigits(4);
    delay(1);
    for (int j = 0; j < 18; j++) {
      displayDigits(2);
      delay(1);
    }
  }
  //delay(3000);
}


// 1->D5;2->D6;3->D7;4->B0;5->B1;6->B2


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
    setDPin(1,HIGH); setDPin(2,LOW);
    clearDPin(3); clearDPin(5);
    clearDPin(4); clearDPin(6);
  } else if (c == 4) {
    setDPin(1,HIGH); setDPin(3,LOW);
    clearDPin(2); clearDPin(6);
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
