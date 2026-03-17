#include <Wire.h>



#define HTU21D_ADDR 0x40
#define TRIGGER_TEMP_MEASURE_HOLD 0xE3
#define TRIGGER_HUMD_MEASURE_HOLD 0xE5

void setup() {
  Wire.begin();
  Wire.setClock(10000); // 10kHz vs normal 100kHz to imrpove stability for 2m wire
  Serial.begin(9600);
  Serial.println("HTUD21D:");
}

void loop() {
  readAndDisplay(TRIGGER_HUMD_MEASURE_HOLD, "Humidity");
  readAndDisplay(TRIGGER_TEMP_MEASURE_HOLD, "Temp");
}

void readAndDisplay(uint8_t command, String type) {
  Wire.beginTransmission(HTU21D_ADDR);
  Wire.write(command);
  Wire.endTransmission();

  delay(60); // Measurement time is ~50ms

  Wire.requestFrom(HTU21D_ADDR, 3);
  if (Wire.available() >= 3) {
    uint8_t msb = Wire.read();
    uint8_t lsb = Wire.read();
    uint8_t checksum = Wire.read();

    if (checkCRC(msb, lsb, checksum)) {
      uint16_t rawData = ((uint16_t)msb << 8) | (lsb ^ 0xFC); // 0xFC to clear status bits
      
      if (type == "Temperature") {
        float temp = -46.85 + (175.72 * (rawData / 65536.0));
        Serial.print("Temp: "); Serial.print(temp); Serial.println(" C");
      } else {
        float humd = -6.0 + (125.0 * (rawData / 65536.0));
        Serial.print("Humd: "); Serial.print(humd); Serial.println(" %");
      }
    } else {
      Serial.println("CRC Error for "); Serial.println(type);
    }
  }
}


// HTU21D/SHT2x use CRC-8 polynomial: x^8 + x^5 + x^4 + 1 (represented as 0x31)
bool checkCRC(uint8_t msb, uint8_t lsb, uint8_t checksum) {
  uint32_t data = ((uint32_t)msb << 16) | ((uint32_t)lsb << 8) | checksum;
  uint32_t divisor = 0x988000; // Polynomial 0x0131 shifted to the left

  for (int i = 0; i < 16; i++) {
    if (data & (uint32_t)1 << (23 - i)) {
      data ^= divisor >> i;
    }
  }
  return (data == 0); // If remainder is 0, CRC is valid
}
