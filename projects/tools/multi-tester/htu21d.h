#ifndef HTU21D_H
#define HTU21D_H

#include <Arduino.h>
#include <Wire.h>

#define HTU21D_ADDR 0x40
#define TRIGGER_TEMP_MEASURE_HOLD 0xE3
#define TRIGGER_HUMD_MEASURE_HOLD 0xE5

namespace HTU21D {

  enum Error { OK, NOT_ENOUGH_DATA_RECEIVED, CRC_ERROR };

  void setup();

  uint16_t readRawTemp();
  uint16_t readRawHum();

  float readTemp();
  float readHum();

  Error getError();

  static bool checkCRC(uint8_t msb, uint8_t lsb, uint8_t checksum);
  
}

#endif
