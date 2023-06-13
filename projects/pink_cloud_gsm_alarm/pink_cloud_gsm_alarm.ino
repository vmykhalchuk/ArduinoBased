const uint8_t pinLedsRGBString = A1;
const uint8_t pinSensorIn = 3;


// RGB LEDs String (START)
#include <FastLED.h>
#define NUM_LEDS 2
// For led chips like WS2812, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
// Clock pin only needed for SPI based chipsets when not using hardware SPI
#define DATA_PIN pinLedsRGBString
//#define CLOCK_PIN 13
// Define the array of leds
CRGB leds[NUM_LEDS];
// RGB LEDs String (END)


void setup() {
  pinMode(pinSensorIn, INPUT_PULLUP);

  {
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed
    leds[0] = CRGB::Red;
    leds[1] = CRGB::Green;
    //CRGB::Blue;
    //CRGB(255,0,255);
    //CRGB::White;
    FastLED.setBrightness(40);
    FastLED.show();
    delay(2000);
    leds[0] = CRGB::Black;
    leds[1] = CRGB::Black;
    //FastLED.setBrightness(80);
    FastLED.show();
  }

  Serial.write("AT\r\n");
}

void loop() {
  if (Serial.available()) Serial.read();
  if (digitalRead(pinSensorIn) == LOW) {
    leds[0] = CRGB::Red;
    FastLED.show();
    // ALARMA!!!!
    Serial.write("ATD+380975411368;\r\n");
    delay(5000);
    leds[0] = CRGB::Black;
    FastLED.show();
  }
}