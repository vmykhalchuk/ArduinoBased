#include "rs485_server.h"
#include "crc8.h"

namespace RS485Server {
  
  Error errorCode = OK;
  
  bool dataReceived = false;
  bool f1 = false, f2 = false;
  bool f3 = false, f4 = false;

  static bool rs_pinDirIsSet = false;
  static int rs_pinDir = 0;

  static bool serial_dataReceivingStarted = false;
  static unsigned long timerMark = 0;
    
  void init(int pinDir) {
    rs_pinDir = pinDir;
    rs_pinDirIsSet = true;
    switchToReceive();
    pinMode(pinDir, OUTPUT);
  }

  void loop() {
    if (!serial_dataReceivingStarted && Serial.available()) {
      serial_dataReceivingStarted = true;
      timerMark = millis();
    }
    
    if (Serial.available() >= 3) {
      
      uint8_t b1 = Serial.read();
      uint8_t b2 = Serial.read();
      uint8_t receivedCRC = Serial.read();
      
      while (Serial.available()) Serial.read(); // flush remaining bytes if present
      serial_dataReceivingStarted = false;
  
      delay(5); // Required 5ms wait before reply
  
      uint8_t payload[2] = {b1, b2};
      uint8_t calcCRC = calculateCRC8(payload, 2);

      uint8_t responseByte = 0;
      bool crcValid = calcCRC == receivedCRC;
      bool dataValid = isDataPacketValid(b1, b2);
      
      if (crcValid && dataValid) {
        responseByte = 0x66; // ACK
        f4 = !(b1 & 0B1);
        f3 = !(b1 & 0B01);
        f2 = !(b1 & 0B001);
        f1 = !(b1 & 0B0001);
        dataReceived = true;
        
      } else {
        
        if (!crcValid) {
          responseByte = 0x96; // CRC_ERR
          errorCode = BAD_CRC;
        } else {
          responseByte = 0x99; // DATA_ERR
          errorCode = BAD_DATA;
        }
      }

      switchToTransmit();
      Serial.write(responseByte);
      switchToReceive();
    }

    if (serial_dataReceivingStarted && (millis() - timerMark >= 100)) {
      // error situation, ignore received data
      errorCode = NOT_ENOUGH_BYTES_RECEIVED;
      while (Serial.available()) Serial.read();
      serial_dataReceivingStarted = false;
    }
  }

  static void switchToReceive() {
    digitalWrite(rs_pinDir, LOW); // switch to Receiving mode
  }
  
  static void switchToTransmit() {
    digitalWrite(rs_pinDir, HIGH); // switch to Transmission mode
  }
  
  static bool isDataPacketValid(uint8_t b1, uint8_t b2) {
    uint8_t b1H = b1 >> 4;
    uint8_t b1L = b1 & 0x0f;
    uint8_t b1Inv = ~b1;
    return (b1H == b1L) && (b1Inv == b2);
  }

}
