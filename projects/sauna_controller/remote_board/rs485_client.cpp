#include "rs485_client.h"
#include "crc8.h"

int RS485Client::errorCode = 0;

static bool rs_pinDirIsSet = false;
static int rs_pinDir = 0;
static bool rs_f1=false, rs_f2=false, rs_f3=false, rs_f4=false;

static RS485Client::StateRS485 currentStateRS485 = RS485Client::IDLE;

static unsigned long timerMark = 0;
static int retryCount = 0;

void RS485Client::init(int pinDir) {
  rs_pinDir = pinDir;
  rs_pinDirIsSet = true;
  pinMode(pinDir, OUTPUT);
  digitalWrite(pinDir, HIGH);
}

void RS485Client::updateFlags(bool f1, bool f2, bool f3, bool f4) {
  rs_f1 = f1;
  rs_f2 = f2;
  rs_f3 = f2;
  rs_f4 = f4;
}

static void RS485Client::sendPacket() {
  if (!rs_pinDirIsSet) {
    RS485Client::errorCode = 0x10;
    return;
  }

  bool f1 = rs_f1, f2 = rs_f2, f3 = rs_f3, f4 = rs_f4;
  
  uint8_t b1 = (f1 << 7) | (f2 << 6) | (f3 << 5) | (f4 << 4) | (!f1 << 3) | (!f2 << 2) | (!f3 << 1) | !f4;
  uint8_t b2 = (!f1 << 7) | (!f2 << 6) | (!f3 << 5) | (!f4 << 4) | (f1 << 3) | (f2 << 2) | (f3 << 1) | f4;
  uint8_t payload[2] = {b1, b2};
  uint8_t crc = calculateCRC8(payload, 2);

  digitalWrite(rs_pinDir, LOW);
  Serial.write(b1);
  Serial.write(b2);
  Serial.write(crc);
  Serial.flush();
  // Performance improvement:
  //   Use my_serial
  //   send byte by byte and check is_uart_idle every time before sending next byte
  // this will also let to move this into separate Loop
  digitalWrite(rs_pinDir, HIGH);
  timerMark = millis();
}

void RS485Client::loop() {
  switch(currentStateRS485) {
    case IDLE:
      if (Serial.available() > 0) {
        while (Serial.available()) Serial.read(); // Clear buffer
        timerMark = millis();
      } else {
        if (millis() - timerMark >= HEARTBEAT_INTERVAL) {
          retryCount = 0;
          sendPacket();
          currentStateRS485 = WAIT_ACK;
        }
      }
      break;

    case WAIT_ACK:
      if (Serial.available() > 0) {
        uint8_t response = Serial.read();
        if (uint8_t(response) == 0x66) {
          currentStateRS485 = IDLE;
          timerMark = millis();
        } else {
          currentStateRS485 = RETRY_DELAY;
          timerMark = millis();
        }
      } else if (millis() - timerMark >= ACK_TIMEOUT) {
        if (retryCount < MAX_RETRIES) {
          retryCount++;
          sendPacket();
        } else {
          currentStateRS485 = IDLE;
          timerMark = millis();
        }
      }
      break;

    case RETRY_DELAY:
      if (millis() - timerMark >= 10) {
        sendPacket();
        currentStateRS485 = WAIT_ACK;
      }
      break;
  }
}
