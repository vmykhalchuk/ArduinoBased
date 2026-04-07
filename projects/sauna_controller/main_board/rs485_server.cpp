#include "rs485_server.h"
#include "crc8.h"

namespace RS485Server {
  
  Error _errorCode = OK;

  Error popError() {
    Error e = _errorCode;
    _errorCode = OK;
    return e;
  }
  
  Error peekError() {
    return _errorCode;
  }

  const uint8_t SWITCH_RX_TO_TX_HOLD = 3;
  const uint8_t SWITCH_TX_TO_RX_WAIT = 3;
  const uint8_t PACKET_TRANSMISSION_MAX_TIME_MS = 30; // !!! depends on baud rate and bytes in single transmission packet
  
  static bool _pinDirIsSet = false;
  static int _pinDir = 0;

  bool _dataRefreshedFlag = false;
  InputData *_in;

  static bool _isDataReceivingStarted = false;
  static unsigned long _timerMark = 0;

  bool peekDataRefreshedFlag() {
    return _dataRefreshedFlag;
  }
  bool popDataRefreshedFlag() {
    bool res = _dataRefreshedFlag;
    _dataRefreshedFlag = false;
    return res;
  }
    
  void init(int pinDir, InputData &inputData) {
    _pinDir = pinDir;
    _pinDirIsSet = true;
    _in = &inputData;
    
    switchToReceive();
    pinMode(_pinDir, OUTPUT);
    delay(100); flushSerialRead();
  }

  void loop() {
    if (!_isDataReceivingStarted && Serial.available()) {
      _isDataReceivingStarted = true;
      _timerMark = millis();
    }
    
    if (Serial.available() >= 3) {
      
      uint8_t b1 = Serial.read();
      uint8_t b2 = Serial.read();
      uint8_t receivedCRC = Serial.read();
      
      flushSerialRead(); // flush remaining bytes if present
      _isDataReceivingStarted = false;
    
      uint8_t payload[2] = {b1, b2};
      uint8_t calcCRC = calculateCRC8(payload, 2);

      uint8_t responseByte = 0;
      bool crcValid = calcCRC == receivedCRC;
      bool dataValid = isDataPacketValid(b1, b2);
      
      if (crcValid && dataValid) {
        responseByte = 0x66; // ACK
        _in->fireAlarm = ! (b1 & 1);
        _in->heatRequest = ! ((b1 >> 1) & 1);
        _in->fanOnRequest = ! ((b1 >> 2) & 1);
        _in->powerOnRequest = ! ((b1 >> 3) & 1);
        _dataRefreshedFlag = true;
        
      } else {
        
        if (!crcValid) {
          responseByte = 0x96; // CRC_ERR
          _errorCode = BAD_CRC;
        } else {
          responseByte = 0x99; // DATA_ERR
          _errorCode = BAD_DATA;
        }
      }

      switchToTransmit();
      Serial.write(responseByte);
      Serial.flush();
      switchToReceive();
    }

    if (_isDataReceivingStarted && (millis() - _timerMark >= PACKET_TRANSMISSION_MAX_TIME_MS)) {
      // error situation, ignore received data
      _errorCode = NOT_ENOUGH_BYTES_RECEIVED;
      flushSerialRead();
      _isDataReceivingStarted = false;
    }
  }

  static bool flushSerialRead() {
    bool r = Serial.available();
    while (Serial.available()) Serial.read();
    return r;
  }

  static void switchToReceive() {
    delay(SWITCH_TX_TO_RX_WAIT); // wait before line stabilizes
    digitalWrite(_pinDir, LOW); // switch to Receiving mode
  }
  
  static void switchToTransmit() {
    digitalWrite(_pinDir, HIGH); // switch to Transmission mode
    delay(SWITCH_RX_TO_TX_HOLD); // let MAX IC to stabilize output
  }
  
  static bool isDataPacketValid(uint8_t b1, uint8_t b2) {
    uint8_t b1H = b1 >> 4;
    uint8_t b1LInv = ~b1 & 0x0f;
    uint8_t b1Inv = ~b1;
    return (b1H == b1LInv) && (b1Inv == b2);
  }

}
