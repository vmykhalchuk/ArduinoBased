#ifndef RS485_CLIENT_H
#define RS485_CLIENT_H

#include <Arduino.h>

namespace RS485Client {

  struct OutputData {
    bool powerOnRequest;
    bool fanOnRequest;
    bool heatRequest;
    bool fireAlarm;
  };

  enum Error { OK, ERROR_INIT, NON_ACK_RECEIVED, ACK_WAIT_TIMEOUT };
  Error popError();
  Error peekError();

  void init(int pinDir, OutputData &outputData);

  // State Machine Loop
  enum State { IDLE, SENDING, WAIT_ACK, RETRY_DELAY };

  void loop();

  // static is essentially private
  static void switchToReceive();
  static void switchToTransmit();
  static void sendPacket();

  static bool flushSerialRead();

  // also resets timer
  static void changeState(State newState);
}


#endif
