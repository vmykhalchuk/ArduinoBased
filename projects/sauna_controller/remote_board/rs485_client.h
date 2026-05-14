#ifndef RS485_CLIENT_H
#define RS485_CLIENT_H

#include <Arduino.h>
#include "clock.h"

namespace RS485Client {

  // Configuration
  const unsigned long HEARTBEAT_INTERVAL = 2000;
  const unsigned long ACK_TIMEOUT = 100;
  const unsigned long SWITCH_RX_TO_TX_HOLD = 3;
  const unsigned long SWITCH_TX_TO_RX_WAIT = 3;
  const unsigned long RETRY_DELAY_INTERVAL = 50;
  const uint8_t MAX_RETRIES = 2;

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

  void tick();

  // static is essentially private
  static void switchToReceive();
  static void switchToTransmit();
  static void sendPacket();

  static bool flushSerialRead();

  // also resets timer
  static void changeState(State newState);
}


#endif
