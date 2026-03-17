#ifndef RS485_CLIENT_H
#define RS485_CLIENT_H

#include <Arduino.h>

namespace RS485Client {

  extern int rs485_errorCode;

  void initRS485Client(int pinDir);

  void rs485ClientUpdateFlags(bool f1, bool f2, bool f3, bool f4);

  // RS485 State Machine Loop

  // Configuration
  const unsigned long HEARTBEAT_INTERVAL = 3000;
  const unsigned long ACK_TIMEOUT = 200;
  const int MAX_RETRIES = 3;

  enum StateRS485 { RS485_IDLE, RS485_SENDING, RS485_WAIT_ACK, RS485_RETRY_DELAY };

  void rs485ClientLoop();
}


#endif