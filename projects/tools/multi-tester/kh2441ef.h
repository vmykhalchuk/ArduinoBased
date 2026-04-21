#ifndef KH2441EF_H
#define KH2441EF_H

#include <Arduino.h>
#include "clock.h"

// Display input->Arduino port: 1->D5;2->D6;3->D7;4->B0;5->B1;6->B2
namespace KH2441EF {

  void displayLoopAsync();
  void displayLoop24ms();
  void displayUpdateForBatch(uint8_t i); // TODO make it private

  void setDisplayBufToFloatWithOneDecimal(float f);
  void setDisplayBuf(uint8_t dig1, uint8_t dig2, uint8_t dig3, uint8_t dig4, bool point);
  void setDisplayBufToErrorMsg();
  void clearDisplayBuf();
  void updateDisplayBufDigitN(uint8_t digitNo, uint8_t v);

  void muteDisplayInstantly(); //mutes display instantly, to resume its operation - call any setDsplay... / updateDisplay...

  //// PRIVATE
  static void lightDSegmentOnlyHigh(uint8_t s);
  static void clearDAll();
  static void setDPinHigh(uint8_t p);
  static void setDPinLow(uint8_t p);
  
}

#endif
