//YWROBOT
//Compatible with the Arduino IDE 1.0
#include <Wire.h> 
//Library version:1.1.2 (https://github.com/johnrickman/LiquidCrystal_I2C) LiquidCrystal I2C
#include <LiquidCrystal_I2C.h>


/**
  SCHEMATIC
 
 input is on A0
 it is pulled up (to +5v) with 400k resistor

 LCD screen is connected:

   LCM1602          Arduino
   Funduino          Nano

   SDA        <-->  A4
   SCL        <-->  A5
 
 */



#if defined(ARDUINO) && ARDUINO >= 100
#define printByte(args)  write(args);
#else
#define printByte(args)  print(args,BYTE);
#endif

uint8_t cc_backslash[8] = { 0x0,0x10,0x8,0x4,0x2,0x1,0x0,0x0};
uint8_t cc_duck[8]  =     { 0x0,0xc,0x1d,0xf,0xf,0x6,0x0,0x0};
uint8_t cc_retarrow[8]  = { 0x1,0x1,0x5,0x9,0x1f,0x8,0x4,0x0};

LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

void setup()
{
  lcd.init();                      // initialize the lcd 
  // Print a message to the LCD.
  lcd.backlight();
  lcd.createChar(1, cc_backslash);
  lcd.createChar(4, cc_duck);
  lcd.createChar(7, cc_retarrow);

  
  lcd.setCursor(1,0);
  lcd.print("v 01 rev 001 "); lcd.printByte(4);
//  lcd.setCursor(2,1);
//  lcd.print("" + String(analogRead(A0)));
//  lcd.print("Ywrobot Arduino!");
//   lcd.setCursor(0,2);
//  lcd.print("Arduino LCM IIC 2004");
//   lcd.setCursor(2,3);
//  lcd.print("Power By Ec-yuan!");
}

String format(int v) {
  int d = 0;
  if (v < 1000) d++;
  if (v < 100) d++;
  if (v < 10) d++;
  String res = "";
  for (int i = 0; i < d; i++) res += "0";
  return res + String(v);
}

int z = 0;

void loop()
{
  // run measurements
  int _min = 3000;
  int _max = 0;
  long sum = 0;
  int c = 5000;

  for (int i = 0; i < c; i++) {
    int v = analogRead(A0);
    if (v < _min) _min = v;
    if (v > _max) _max = v;
    sum += v;
  }
  
  lcd.setCursor(0,1);
  long avg = sum / c;
  
  String zStr; // |/-\
  // -------------------
  if (z == 0) {
    zStr = "|";
  } else if (z == 1) {
    zStr = "/";
  } else if (z == 2) {
    zStr = "-";
  } else if (z == 3) {
    zStr = "\x01";
  }

  z = ++z % 4;
  
  lcd.print(zStr + " " + format(_min) + "<" + format(avg) + "<" + format(_max) + "       ");
  //delay(100);
}
