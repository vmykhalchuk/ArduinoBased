#include "htu21d.h"

namespace HTU21D {

  void setup() {
    Wire.begin();
    Wire.setClock(10000); // 10kHz vs normal 100kHz to imrpove stability for long wire
  }

  Error conversionError = OK;

  Error getError() {
    return conversionError;
  }

  // 0 - temp; 1 - hum
  uint16_t readRawValue(uint8_t type) {
    conversionError = OK;
    Wire.beginTransmission(HTU21D_ADDR);
    Wire.write(type == 0 ? TRIGGER_TEMP_MEASURE_HOLD : TRIGGER_HUMD_MEASURE_HOLD);
    Wire.endTransmission();
  
    delay(60); // Measurement time is ~50ms
  
    Wire.requestFrom(HTU21D_ADDR, 3);
    if (Wire.available() < 3) {
      conversionError = NOT_ENOUGH_DATA_RECEIVED;
    } else {
      uint8_t msb = Wire.read();
      uint8_t lsb = Wire.read();
      uint8_t checksum = Wire.read();
  
      if (!checkCRC(msb, lsb, checksum)) {
        conversionError = CRC_ERROR;
      } else {
        uint16_t rawData = ((uint16_t)msb << 8) | (lsb ^ 0xFC); // 0xFC to clear status bits
        return rawData;
      }
    }
    return 0;
  }
  
  uint16_t readRawTemp() {
    return readRawValue(0);
  }
  
  uint16_t readRawHum() {
    return readRawValue(1);
  }

  float readTemp() {
    uint16_t rawData = readRawValue(0);
    if (conversionError != OK) {
      return -99.0;
    }
    return -46.85 + (175.72 * (rawData / 65536.0));
  }
  
  float readHum() {
    uint16_t rawData = readRawValue(0);
    if (conversionError != OK) {
      return -99.0;
    }
    return -6.0 + (125.0 * (rawData / 65536.0));
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

}
