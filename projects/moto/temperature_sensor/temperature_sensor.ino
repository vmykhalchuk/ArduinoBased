#include <OneWire.h>
#include <DallasTemperature.h>

// Sensor Data Wire (externally pulled-up with 4.7k)
#define ONE_WIRE_BUS 4

OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

void setup() {
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  
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
  delay(3000);
}



void displayDigits(int c) {
  c = c % 3; //12;
  if (c == 0) {
    digitalWrite(4+1, LOW); digitalWrite(4+2, HIGH);
    digitalWrite(4+3, HIGH); digitalWrite(4+5, LOW);
    
    digitalWrite(4+4, LOW); digitalWrite(4+6, LOW);
  } else if (c == 1) {
    digitalWrite(4+1, LOW); digitalWrite(4+3, HIGH);
    digitalWrite(4+2, LOW); digitalWrite(4+6, HIGH);

    digitalWrite(4+4, LOW); digitalWrite(4+6, LOW);
  } else if (c == 2) {
    digitalWrite(4+1, LOW); digitalWrite(4+2, LOW);
    digitalWrite(4+3, LOW); digitalWrite(4+4, LOW);
    digitalWrite(4+5, LOW); digitalWrite(4+6, LOW);
  }
}
