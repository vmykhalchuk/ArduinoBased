#ifndef KH2441EF_H
#define KH2441EF_H

#include <Arduino.h>

namespace KH2441EF {

  // Display input->Arduino port: 1->D5;2->D6;3->D7;4->B0;5->B1;6->B2

  void displayLoopAsync(uint16_t _m);
  void displayLoop24ms();
  void displayUpdateForBatch(uint8_t i);

  void setDisplayBufToFloatWithOneDecimal(float f);
  void setDisplayBuf(uint8_t dig1, uint8_t dig2, uint8_t dig3, uint8_t dig4, bool point);
  void setDisplayBufToErrorMsg();
  void clearDisplayBuf();
  void updateDisplayBufDigitN(uint8_t digitNo, uint8_t v);

  //// PRIVATE
  static void lightDSegmentOnlyHigh(uint8_t s);
  static void clearDAll();
  static void setDPinHigh(uint8_t p);
  static void setDPinLow(uint8_t p);
  
}

#endif
