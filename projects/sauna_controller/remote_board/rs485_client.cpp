#include "rs485_client.h"
#include "crc8.h"


namespace RS485Client {

  Error _errorCode = OK;

  Error popError() {
    Error e = _errorCode;
    _errorCode = OK;
    return e;
  }
  
  Error peekError() {
    return _errorCode;
  }

  // Configuration
  const unsigned long HEARTBEAT_INTERVAL = 6000; // orig: 3000
  const unsigned long ACK_TIMEOUT = 800; //orig: 200
  const unsigned long SWITCH_RX_TO_TX_HOLD = 30; //orig: 3
  const unsigned long SWITCH_TX_TO_RX_WAIT = 20; //orig: 3
  const unsigned long RETRY_DELAY_INTERVAL = 200; // orig: 50
  const uint8_t MAX_RETRIES = 3;

  // Variables
  static bool _pinDirIsSet = false;
  static int _pinDir = 0;
  
  static bool _f1=false, _f2=false, _f3=false, _f4=false;
  
  static State _currentState = IDLE;
  static unsigned long _timerMark = 0;
  static uint8_t _retryCount = 0;

  void init(int pinDir) {
    _pinDir = pinDir;
    _pinDirIsSet = true;
    switchToTransmit(); // to prevent Main board from receiving random noise
    pinMode(_pinDir, OUTPUT);
    delay(100); flushSerialRead();
    changeState(IDLE);
  }
  
  void updateFlags(bool f1, bool f2, bool f3, bool f4) {
    _f1 = f1;
    _f2 = f2;
    _f3 = f2;
    _f4 = f4;
  }
  
  static void switchToReceive() {
    delay(SWITCH_TX_TO_RX_WAIT); // wait before line stabilizes after previous transmission
    digitalWrite(_pinDir, LOW); // switch to Receiving mode
  }
  
  static void switchToTransmit() {
    digitalWrite(_pinDir, HIGH); // switch to Transmission mode
    delay(SWITCH_RX_TO_TX_HOLD); // let MAX IC to stabilize output
  }
  
  static void sendPacket() {
    if (!_pinDirIsSet) {
      _errorCode = ERROR_INIT;
      return;
    }
  
    uint8_t f1 = _f1 ? 1 : 0, f2 = _f2 ? 1 : 0, f3 = _f3 ? 1 : 0, f4 = _f4 ? 1 : 0;
    uint8_t f1Inv = _f1 ? 0 : 1, f2Inv = _f2 ? 0 : 1, f3Inv = _f3 ? 0 : 1, f4Inv = _f4 ? 0 : 1;
    
    uint8_t b1 = (f1 << 7) | (f2 << 6) | (f3 << 5) | (f4 << 4) | (f1Inv << 3) | (f2Inv << 2) | (f3Inv << 1) | f4Inv;
             
    uint8_t b2 = (f1Inv << 7) | (f2Inv << 6) | (f3Inv << 5) | (f4Inv << 4) | (f1 << 3) | (f2 << 2) | (f3 << 1) | f4;
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
    switchToReceive();
  }

  static bool flushSerialRead() {
    bool res = Serial.available();
    while (Serial.available()) Serial.read();
    return res;
  }

  static void changeState(State newState) {
    _currentState = newState;
    _timerMark = millis();
  }
  
  void loop() {
    switch(_currentState) {
      case IDLE:
        if (Serial.available() > 0) {
          flushSerialRead();
          changeState(IDLE); // restart timer because of some noise on line
        } else {
          if (millis() - _timerMark >= HEARTBEAT_INTERVAL) {
            sendPacket();
            _retryCount = 0;
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
            _errorCode = NON_ACK_RECEIVED;
            changeState(RETRY_DELAY);
          }
        } else if (millis() - _timerMark >= ACK_TIMEOUT) {
          _errorCode = ACK_WAIT_TIMEOUT;
          if (_retryCount < MAX_RETRIES) {
            _retryCount++;
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
        if (millis() - _timerMark >= RETRY_DELAY_INTERVAL) {
          sendPacket();
          changeState(WAIT_ACK);
        }
        break;
    }
  }
}
