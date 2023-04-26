#include <SoftwareSerial.h>

const byte rxPin = 2;
const byte txPin = 5;

const byte crPin = 9;

const byte demoTxPin = 3;

// Set up a new SoftwareSerial object
SoftwareSerial mySerial (rxPin, txPin);

unsigned long tmstmp;
unsigned long tmstmpMicro;
unsigned long microValue = 50;
bool demoTxPinOn = false;
byte c = 0;


/* Code to pulse pin 3 with a modulated signal
* Can be used to drive an IR LED to keep a TSOP IR reciever happy
* This allows you to use a modulated reciever and a continious beam detector
* By Mike Cook Nov 2011 - Released under the Open Source licence
*/
 volatile byte pulse = 0;
 volatile byte state = 0;

/*ISR(TIMER2_COMPB_vect){  // Interrupt service routine to pulse the modulated pin 3
  if (state != 0) {
      pulse++;
    if(pulse >= 8) { // change number for number of modulation cycles in a pulse
      pulse = 0;
      TCCR2A ^= _BV(COM2B1); // toggle pin 3 enable, turning the pin on and off
    }
  }
}*/

void setIrModOutput() {  // sets pin 3 going at the IR modulation rate
  pinMode(3, OUTPUT);
/* 
  TCCR2A = _BV(COM2B1) | _BV(WGM21) | _BV(WGM20); // Just enable output on Pin 3 and disable it on Pin 11
  TCCR2B = _BV(WGM22) | _BV(CS22);
  OCR2A = 51; // defines the frequency 51 = 38.4 KHz, 54 = 36.2 KHz, 58 = 34 KHz, 62 = 32 KHz
  OCR2B = 26;  // deines the duty cycle - Half the OCR2A value for 50%
  TCCR2B = TCCR2B & 0b00111000 | 0x2; // select a prescale value of 8:1 of the system clock
  */
}

void setup() {
  pinMode(3, OUTPUT);
  
  Serial.begin(9600);
  //mySerial.begin(1200);
  //tone(crPin, 38000);

  Serial.println("Started!");
  tmstmp = millis();
  tmstmpMicro = micros();

  /*setIrModOutput();
  TIMSK2 = _BV(OCIE2B); // Output Compare Match B Interrupt Enable
  */
}

bool rxPinState = false;
int colCount = 0;
void loop() {
  unsigned long _millis = millis();
  
  /*if (mySerial.available()) {
    Serial.println(mySerial.read(), HEX);
  }*/

  bool rxPinV = digitalRead(rxPin) ? true : false;
  if (rxPinV != rxPinState) {
    if (rxPinV) {
      Serial.print('^');
    } else {
      Serial.print('_');
    }
    colCount++;
    if (colCount >= 80) {
      Serial.println();
      colCount = 0;
    }
    rxPinState = rxPinV;
  }

  if ((_millis - tmstmp) > 3) {
    noTone(3);
  }
  
  if ((_millis - tmstmp) > 1000) {
    Serial.print('.'); colCount++;
    tone(3, 38000);
    if (colCount >= 80) {
      Serial.println();
      colCount = 0;
    }
    tmstmp = _millis;
  }

  // 14 microseconds

  

  /*unsigned long _micros = micros();
  if ((_micros - tmstmpMicro) > microValue) {
    microValue = 800 + _micros%20*80;

    if (state > 0) {
      digitalWrite(3, LOW);
      state = 0;
    } else {
      digitalWrite(3, LOW);
      state = 10;
    }
    
    tmstmpMicro = _micros;
  }*/

  /*if ((millis() - tmstmp) > 1000) {
    Serial.print('.');
    mySerial.write(c++);
    tmstmp = millis();
  }*/
}
