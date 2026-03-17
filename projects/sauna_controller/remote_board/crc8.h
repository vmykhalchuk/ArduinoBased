#ifndef CRC8_CALC_H
#define CRC8_CALC_H

#include <Arduino.h>

uint8_t calculateCRC8(uint8_t *data, size_t len);

#endif