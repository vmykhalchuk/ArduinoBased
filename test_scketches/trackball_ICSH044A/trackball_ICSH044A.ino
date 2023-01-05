const uint8_t pinLCDHighlightPWM = 9;

const uint8_t pinTrackballUp = 11;
const uint8_t pinTrackballDown = 12;


// LCD (START)
//YWROBOT
//Compatible with the Arduino IDE 1.0
#include <Wire.h>
//Library version:1.1.2 (https://github.com/johnrickman/LiquidCrystal_I2C) LiquidCrystal I2C
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display
#if defined(ARDUINO) && ARDUINO >= 100
#define printByte(args)  write(args);
#else
#define printByte(args)  print(args,BYTE);
#endif
// LCD (END)

void setup() {
  pinMode(pinLCDHighlightPWM, OUTPUT);
  analogWrite(pinLCDHighlightPWM, 255);

  lcd.init(); // initialize lcd
  // Print a message to the LCD.
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(F("Test Trackball"));
  lcd.setCursor(0,1);
  lcd.print(F("05Jan2023"));
  delay(3000);
  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print(F("Up:     Dn:     "));
  lcd.setCursor(0,1);
  lcd.print(F("ms:"));
}

bool trackballUpPulseStarted = false;
bool trackballDownPulseStarted = false;
uint16_t trackballUpPulseStartMillis;
uint16_t trackballDownPulseStartMillis;

uint16_t upCounter = 0;
uint16_t downCounter = 0;

void loop() {
  uint16_t _m = millis();
  bool upState = digitalRead(pinTrackballUp);
  bool downState = digitalRead(pinTrackballDown);

  if (trackballUpPulseStarted != upState) {
    if (upState) {
      // pulse started
      trackballUpPulseStarted = true;
      trackballUpPulseStartMillis = _m;
    } else {
      // pulse finished
      trackballUpPulseStarted = false;
      uint16_t resMs = _m - trackballUpPulseStartMillis;
      upCounter++;
      lcd.setCursor(3,0);
      lcd.print(upCounter);
      lcd.setCursor(3,1);
      lcd.print(resMs);
      lcd.print("    ");
    }
  }

  if (trackballDownPulseStarted != downState) {
    if (downState) {
      // pulse started
      trackballDownPulseStarted = true;
      trackballDownPulseStartMillis = _m;
    } else {
      // pulse finished
      trackballDownPulseStarted = false;
      uint16_t resMs = _m - trackballDownPulseStartMillis;
      downCounter++;
      lcd.setCursor(3+8,0);
      lcd.print(downCounter);
      lcd.setCursor(3,1);
      lcd.print(resMs);
      lcd.print("    ");
    }
  }
}