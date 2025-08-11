#include <OneWire.h>
#include <DallasTemperature.h>

// Sensor Data Wire (externally pulled-up with 4.7k)
#define ONE_WIRE_BUS 4

OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors($oneWire);

void setup() {
  Serial.begin(9600);
  sensors.begin();
}

void loop() {
  sensors.requestTemperatures();
  Serial.print("Celcius temperature: ");
  Serial.println(sensors.getTempCByIndex(0));
  delay(1000);
}