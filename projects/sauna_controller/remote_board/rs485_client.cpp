#include "rs485_client.h"
#include "crc8.h"


namespace RS485Client {

  Error errorCode = OK;

  Error popError() {
    Error e = errorCode;
    errorCode = OK;
    return e;
  }
  
  Error peekError() {
    return errorCode;
  }

  static bool rs_pinDirIsSet = false;
  static int rs_pinDir = 0;
  
  static bool rs_f1=false, rs_f2=false, rs_f3=false, rs_f4=false;
  
  static State currentState = 0;
  static unsigned long timerMark = 0;
  static uint8_t retryCount = 0;

  void init(int pinDir) {
    rs_pinDir = pinDir;
    rs_pinDirIsSet = true;
    switchToTransmit(); // to prevent Main board from receiving random noise
    pinMode(rs_pinDir, OUTPUT);
    changeState(IDLE);
  }
  
  void updateFlags(bool f1, bool f2, bool f3, bool f4) {
    rs_f1 = f1;
    rs_f2 = f2;
    rs_f3 = f2;
    rs_f4 = f4;
  }
  
  static void switchToReceive() {
    digitalWrite(rs_pinDir, LOW); // switch to Receiving mode
  }
  
  static void switchToTransmit() {
    digitalWrite(rs_pinDir, HIGH); // switch to Transmission mode
    delay(3);
  }
  
  static void sendPacket() {
    if (!rs_pinDirIsSet) {
      errorCode = ERROR_INIT;
      return;
    }
  
    bool f1 = rs_f1, f2 = rs_f2, f3 = rs_f3, f4 = rs_f4;
    
    uint8_t b1 = (f1 << 7) | (f2 << 6) | (f3 << 5) | (f4 << 4) | (!f1 << 3) | (!f2 << 2) | (!f3 << 1) | !f4;
    uint8_t b2 = (!f1 << 7) | (!f2 << 6) | (!f3 << 5) | (!f4 << 4) | (f1 << 3) | (f2 << 2) | (f3 << 1) | f4;
    uint8_t payload[2] = {b1, b2};
    uint8_t crc = calculateCRC8(payload, 2);
  
    // Performance improvement:
    //   Use my_serial
    //   send byte by byte and check is_uart_idle every time before sending next byte
    // this will also let to move this into separate Loop

    switchToTransmit();
    Serial.write(b1);
    Serial.write(b2);
    Serial.write(crc);
    Serial.flush(); // wait till physical transfer completes
    delay(3);
    switchToReceive();
  }

  static void flushSerialRead() {
    if (Serial.available()) {
      while (Serial.available()) Serial.read();
    }
  }

  static void changeState(State newState) {
    currentState = newState;
    timerMark = millis();
  }
  
  void loop() {
    switch(currentState) {
      case IDLE:
        if (Serial.available() > 0) {
          flushSerialRead();
          changeState(currentState); // restart timer because of some noise on line
        } else {
          if (millis() - timerMark >= HEARTBEAT_INTERVAL) {
            sendPacket();
            retryCount = 0;
            changeState(WAIT_ACK);
          }
        }
        break;
  
      case WAIT_ACK:
        if (Serial.available() > 0) {
          uint8_t response = Serial.read();
          if (uint8_t(response) == 0x66) {
            changeState(IDLE);
          } else {
            errorCode = NON_ACK_RECEIVED;
            changeState(RETRY_DELAY);
          }
        } else if (millis() - timerMark >= ACK_TIMEOUT) {
          errorCode = ACK_WAIT_TIMEOUT;
          if (retryCount < MAX_RETRIES) {
            retryCount++;
            sendPacket();
            changeState(WAIT_ACK);
          } else {
            changeState(IDLE);
          }
        } else {
          // wait for timeout
        }
        break;
  
      case RETRY_DELAY:
        flushSerialRead();
        if (millis() - timerMark >= 50) {
          sendPacket();
          changeState(WAIT_ACK);
        }
        break;
    }
  }
}
