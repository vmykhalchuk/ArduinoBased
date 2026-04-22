#ifndef KH2441EF_H
#define KH2441EF_H

#include <Arduino.h>
#include "clock.h"

// Display input->Arduino port: 1->D5;2->D6;3->D7;4->B0;5->B1;6->B2
namespace KH2441EF {

  const uint8_t S_BLANK = 0x10;
  const uint8_t S_E = 0x11;
  const uint8_t S_r = 0x12;
  const uint8_t S_P = 0x13;
  const uint8_t S_SEL1 = 0x14;
  const uint8_t S_SEL2 = 0x15;
  const uint8_t S_c = 0x16;
  const uint8_t S_C = 0x17;
  const uint8_t S_t = 0x18;
  const uint8_t S_H = 0x19;
  const uint8_t S_h = 0x20;
  const uint8_t S_UND = 0x21;

  // use either tick() or displayLoop24ms()
  void tick(); // non-blocking
  
  void displayLoop24ms(); // blocking for 24ms
  void displayUpdateForBatch(uint8_t i); // TODO make it private

  void setDisplayBufToInt(uint16_t i);
  void setDisplayBufToFloatWithOneDecimal(float f);
  void setDisplayBuf(uint8_t dig1, uint8_t dig2, uint8_t dig3, uint8_t dig4, bool point);
  void setDisplayBufToErrorMsg();
  void clearDisplayBuf();
  void updateDisplayBufDigitN(uint8_t digitNo, uint8_t v);

  // clear buf and disable ports so display goes dark (useful to prevent some segments to light bright when heavy operation is happening and tick() is not called for extended period)
  void muteDisplayInstantly(); //mutes display instantly, to resume its operation - call any setDsplay... / updateDisplay...

  //// PRIVATE
  static void lightDSegmentOnlyHigh(uint8_t s);
  static void clearDAll();
  static void setDPinHigh(uint8_t p);
  static void setDPinLow(uint8_t p);
  
}

#endif
