const uint8_t pinLCDHighlightPWM = 9;

const uint8_t pinTrackballUp = 8;
const uint8_t pinTrackballDown = 6;


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

bool trackballUpLastState;
bool trackballDownLastState;

uint16_t upCounter = 0;
uint16_t downCounter = 0;

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
  lcd.print(F("2023Jan09"));
  delay(3000);
  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print(F("Up:     Dn:     "));

  trackballUpLastState = digitalRead(pinTrackballUp);
  trackballDownLastState = digitalRead(pinTrackballDown);
}

void loop() {
  bool upState = digitalRead(pinTrackballUp);
  bool downState = digitalRead(pinTrackballDown);

  if (trackballUpLastState != upState) {
    trackballUpLastState = upState;
    lcd.setCursor(3,0);
    lcd.print(++upCounter);
  }
  if (trackballDownLastState != downState) {
    trackballDownLastState = downState;
    lcd.setCursor(3+8,0);
    lcd.print(++downCounter);
  }
}