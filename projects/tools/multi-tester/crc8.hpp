#ifndef CRC8_CALC_H
#define CRC8_CALC_H

#include <Arduino.h>

class CRC8 final {
  private:
    CRC8() {};
  public:
    static uint8_t calculate(uint8_t *data, size_t len) {
      uint8_t crc = 0x00;
      for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
          if (crc & 0x80) crc = (crc << 1) ^ 0x07;
          else crc <<= 1;
        }
      }
      return crc;
    }
};

#endif
