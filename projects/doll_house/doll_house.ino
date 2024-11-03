const uint8_t pinBtn0 = A0;
const uint8_t pinBtn1 = A1;

const uint8_t pinLedsRGBString = A3;

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

void setup()
{
  Serial.begin(9600);
  while(!Serial) {
    // some boards need to wait to ensure access to serial over USB
  }

  {
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed
    leds[0] = CRGB::White;
    leds[1] = CRGB::Green;
    //leds[2] = CRGB::Blue;
    //leds[3] = CRGB(255,0,255);//CRGB::Blue;
    //leds[4] = CRGB::White;
    FastLED.setBrightness(40);
    FastLED.show();
    delay(2000);
    leds[0] = CRGB::Black;
    leds[1] = CRGB::Black;
    FastLED.setBrightness(225);
    FastLED.show();
  }
}

bool light0IsOn = false;
bool light1IsOn = false;

void loop() {
  if (digitalRead(pinBtn0) == HIGH) {
    while(digitalRead(pinBtn0) == HIGH) {
    }
    light0IsOn = !light0IsOn;
    if (light0IsOn) {
      leds[0] = CRGB::White;
    } else {
      leds[0] = CRGB::Black;
    }
    FastLED.show();
  }

  if (digitalRead(pinBtn1) == HIGH) {
    while(digitalRead(pinBtn1) == HIGH) {
    }
    light1IsOn = !light1IsOn;
    if (light1IsOn) {
      leds[1] = CRGB::White;
    } else {
      leds[1] = CRGB::Black;
    }
    FastLED.show();
  }
}