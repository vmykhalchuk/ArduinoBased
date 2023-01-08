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

bool trackballUpPosition;
bool trackballDownPosition;
uint16_t trackballUpMs;
uint16_t trackballDownMs;

void setup() {
  pinMode(pinLCDHighlightPWM, OUTPUT);
  analogWrite(pinLCDHighlightPWM, 255);

  lcd.init(); // initialize lcd
  // Print a message to the LCD.
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(F("TestV2 Trackball"));
  lcd.setCursor(0,1);
  lcd.print(F("08Jan2023"));
  delay(3000);
  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print(F("Up:     Dn:     "));
  lcd.setCursor(0,1);
  lcd.print(F("ms:"));

  trackballUpPosition = digitalRead(pinTrackballUp);
  trackballDownPosition = digitalRead(pinTrackballDown);
  trackballUpMs = trackballDownMs = millis();
}

uint16_t upCounter = 0;
uint16_t downCounter = 0;

void loop() {
  uint16_t _m = millis();

  if (trackballUpPosition != digitalRead(pinTrackballUp)) {
    // pos changed
    uint16_t resMs = _m - trackballUpMs;
    trackballUpPosition != trackballUpPosition;
    trackballUpMs = _m;
    upCounter++;
    lcd.setCursor(3,0);
    lcd.print(upCounter);
    lcd.setCursor(3,1);
    lcd.print(resMs);
    lcd.print("    ");
  }
  if (trackballDownPosition != digitalRead(pinTrackballDown)) {
    // pos changed
    uint16_t resMs = _m - trackballDownMs;
    trackballDownPosition != trackballDownPosition;
    trackballDownMs = _m;
    downCounter++;
    lcd.setCursor(3+8,0);
    lcd.print(downCounter);
    lcd.setCursor(3,1);
    lcd.print(resMs);
    lcd.print("    ");
  }
}