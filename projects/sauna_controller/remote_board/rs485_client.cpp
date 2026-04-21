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

  // Variables
  static bool _pinDirIsSet = false;
  static int _pinDir = 0;
  
  OutputData *_out;
  
  static State _currentState = IDLE;
  static unsigned long _timerMark = 0;
  static uint8_t _retryCount = 0;

  void init(int pinDir, OutputData &outputData) {
    _pinDir = pinDir;
    _out = &outputData;

    _pinDirIsSet = true;
    switchToTransmit(); // to prevent Main board from receiving random noise
    pinMode(_pinDir, OUTPUT);
    delay(100); flushSerialRead();
    changeState(IDLE);
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
  
    uint8_t f1 = _out->powerOnRequest ? 1 : 0,
            f2 = _out->fanOnRequest ? 1 : 0,
            f3 = _out->heatRequest ? 1 : 0,
            f4 = _out->fireAlarm ? 1 : 0;
    
    uint8_t b1 = (f1 << 7) | (f2 << 6) | (f3 << 5) | (f4 << 4) |
                 (!f1 << 3) | (!f2 << 2) | (!f3 << 1) | !f4;
             
    uint8_t b2 = (!f1 << 7) | (!f2 << 6) | (!f3 << 5) | (!f4 << 4) |
                 (f1 << 3) | (f2 << 2) | (f3 << 1) | f4;
                 
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
    // FIXME Unblock this part of code (flush is blocking operation) (use my_serial)
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
    _timerMark = ClockLR::now;
  }
  
  void tick() {
    switch(_currentState) {
      case IDLE:
        if (Serial.available() > 0) {
          flushSerialRead();
          changeState(IDLE); // restart timer because of some noise on line
        } else {
          if (ClockLR::isElapsed(_timerMark, HEARTBEAT_INTERVAL)) {
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
        } else if (ClockLR::isElapsed(_timerMark, ACK_TIMEOUT)) {
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
        if (ClockLR::isElapsed(_timerMark, RETRY_DELAY_INTERVAL)) {
          sendPacket();
          changeState(WAIT_ACK);
        }
        break;
    }
  }
}
