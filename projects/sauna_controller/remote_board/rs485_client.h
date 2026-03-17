#ifndef RS485_CLIENT_H
#define RS485_CLIENT_H

#include <Arduino.h>

namespace RS485Client {

  extern int errorCode;

  void init(int pinDir);

  void updateFlags(bool f1, bool f2, bool f3, bool f4);

  // Configuration
  const unsigned long HEARTBEAT_INTERVAL = 3000;
  const unsigned long ACK_TIMEOUT = 200;
  const int MAX_RETRIES = 3;

  // RS485 State Machine Loop
  enum StateRS485 { IDLE, SENDING, WAIT_ACK, RETRY_DELAY };

  //extern unsigned long timerMark;

  void loop();

  // static is essentially private
  static void sendPacket();
}


#endif
