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
    Serial.print(d10);
    Serial.print(",");
    Serial.print(d0);
    Serial.println("]");
  }
  else
  {
    Serial.println("Error: Could not read temperature data");
  }
  delay(1000);
}
