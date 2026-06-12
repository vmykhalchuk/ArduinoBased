#ifndef KH2441EF_H
#define KH2441EF_H

#include <Arduino.h>
#include "clock.h"

// Display input->Arduino port: 1->5(D5);2->6(D6);3->7(D7);4->8(B0);5->9(B1);6->10(B2)
namespace KH2441EF {

  const uint8_t S_A = 0x10;
  const uint8_t S_b = 0x11;
  const uint8_t S_c = 0x12;
  const uint8_t S_d = 0x13;
  const uint8_t S_E = 0x14;
  const uint8_t S_F = 0x15;
  const uint8_t S_BLANK = 0x20;
  const uint8_t S_C = 0x21;
  const uint8_t S_h = 0x22;
  const uint8_t S_H = 0x23;
  const uint8_t S_P = 0x24;
  const uint8_t S_r = 0x25;
  const uint8_t S_S = 0x26;
  const uint8_t S_t = 0x27;
  const uint8_t S_u = 0x28;
  const uint8_t S_SEL1 = 0x29;
  const uint8_t S_SEL2 = 0x30;
  const uint8_t S_UND = 0x31;   // _
  const uint8_t S_MINUS = 0x32;
  const uint8_t S_QM = 0x33;    // ?

  // use either tick() or displayLoop24ms()
  void tick(); // non-blocking
  
  void displayLoop24ms(); // blocking for 24ms
  void displayUpdateForBatch(uint8_t i); // TODO make it private

  // clear and set int value
  void setDisplayBufToInt(uint16_t i);
  // clear and set int value
  void setDisplayBufToInt(uint16_t i, bool displayLeadingZeroes);
  // clear and set int value as Hex
  void setDisplayBufToIntAsHex(uint16_t i, bool displayLeadingZeroes);
  // clear and set float value (only one decimal 0.0 - 199.9)
  void setDisplayBufToFloatWithOneDecimal(float f);
  // clear and set digit by digit
  void setDisplayBuf(uint8_t dig1, uint8_t dig2, uint8_t dig3, uint8_t dig4, bool point);
  // update certain digit (masks current value)
  void updateDisplayBufDigit(uint8_t digitNo, uint8_t digValue);
  // sets point
  void updateDisplayBufPoint();
  // clear and set Err message on display
  void setDisplayBufToErrorMsg();
  // clear display
  void clearDisplayBuf();
  
  void _updateDisplayBufDigitN(uint8_t digitNo, uint8_t v);

  // clear buf and disable ports so display goes dark (useful to prevent some segments to light bright when heavy operation is happening in between tick() calls)
  void muteDisplayInstantly(); //mutes display instantly, to resume its operation - call any setDsplay... / updateDisplay...
  
}

#endif
