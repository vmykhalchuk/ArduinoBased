#ifndef RS485_SERVER_H
#define RS485_SERVER_H

#include <Arduino.h>

namespace RS485Server {

  struct InputData {
    bool powerOnRequest;
    bool fanOnRequest;
    bool heatRequest;
    bool fireAlarm;
  };

  enum Error { OK, NOT_ENOUGH_BYTES_RECEIVED, BAD_CRC, BAD_DATA };
  Error popError();
  Error peekError();
  
  // dataRefreshedFlag
  //represents fact that new transmission received)
  // in other words - it signals caller code that new data
  // is available to read from f1 - f4
  // (doesn't necessarily mean the f1-f4 flags have changed)
  bool peekDataRefreshedFlag();
  bool popDataRefreshedFlag();
  
  //extern bool f1, f2, f3, f4;
  void init(int pinDir, InputData &inputData);
  void loop();

  // Static is essentially private

  static bool flushSerialRead();
  
  static void switchToReceive();
  static void switchToTransmit();
  
  /*
   * Validate if data packet conforms communication template (see communication.txt)
   */
  static bool isDataPacketValid(uint8_t b1, uint8_t b2);
}


#endif
