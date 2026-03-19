#ifndef RS485_CLIENT_H
#define RS485_CLIENT_H

#include <Arduino.h>

namespace RS485Client {

  enum Error { OK, ERROR_INIT, NON_ACK_RECEIVED, ACK_WAIT_TIMEOUT };
  Error popError();
  Error peekError();

  void init(int pinDir);

  void updateFlags(bool f1, bool f2, bool f3, bool f4);


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
