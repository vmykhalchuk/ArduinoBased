const uint8_t pinLedsCandlesString = 9;
const uint8_t pinLedsRGBString = A3;


// RGB LEDs String (START)
#include <FastLED.h>
#define NUM_LEDS 5
// For led chips like WS2812, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
// Clock pin only needed for SPI based chipsets when not using hardware SPI
#define DATA_PIN pinLedsRGBString
//#define CLOCK_PIN 13
// Define the array of leds
CRGB leds[NUM_LEDS];
// RGB LEDs String (END)


uint16_t candleTimerMs = 0;
uint16_t candleDelaySetMs = 10;
uint16_t blinkingTimerMs = 0;
uint16_t blinkingDelaySetMs = 0;

void setup()
{
  pinMode(pinLedsCandlesString, OUTPUT);

  {
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed
    leds[0] = CRGB::Red;
    leds[1] = CRGB::Green;
    leds[2] = CRGB::Blue;
    leds[3] = CRGB(255,0,255);//CRGB::Blue;
    leds[4] = CRGB::White;
    FastLED.setBrightness(40);
    FastLED.show();
    delay(2000);
    leds[0] = CRGB::Black;
    leds[1] = CRGB::Black;
    leds[2] = CRGB::Black;
    leds[3] = CRGB::Black;
    leds[4] = CRGB::Black;
    FastLED.setBrightness(80);
    FastLED.show();
  }

  candleTimerMs = (uint16_t) millis();
  blinkingTimerMs = candleTimerMs;
  candleDelaySetMs = 5000;
  blinkingDelaySetMs = 10000;
}

uint8_t candleBrightness = 0;
bool candleBrightnessIncr = true;

uint8_t ledBlinked = 255; // 255 - none

void loop() {
  uint16_t _m = (uint16_t) millis();
  if ((_m - candleTimerMs) >= candleDelaySetMs) {
    if (candleBrightnessIncr) {
      if (candleBrightness <= 70) {                     // orig: 250
        candleBrightness += 1;                          // orig: 5
      } else {
        candleBrightness = 75;                          // orig 255
        candleBrightnessIncr = false;
      }
    } else {
      if (candleBrightness > 0) {                       // orig: 40
        candleBrightness -= 1;                          // orig: 5
      } else {
        candleBrightness = 0;                           // orig: 40
        candleBrightnessIncr = true;
      }
    }
    analogWrite(pinLedsCandlesString, candleBrightness);

    candleTimerMs = _m; candleDelaySetMs = 40;
  }

  if ((_m - blinkingTimerMs) >= blinkingDelaySetMs) {
    if (ledBlinked == 255) {
      ledBlinked = random(5);
      if (ledBlinked > 4) ledBlinked = 4;
      leds[ledBlinked] = CRGB::White;
      FastLED.show();

      blinkingTimerMs = _m;
       blinkingDelaySetMs = 30;
    } else {
      // turn off LED blinked
      ledBlinked = 255;
      leds[0] = CRGB::Black;
      leds[1] = CRGB::Black;
      leds[2] = CRGB::Black;
      leds[3] = CRGB::Black;
      leds[4] = CRGB::Black;
      FastLED.show();

      blinkingTimerMs = _m;
      blinkingDelaySetMs = 500 + random(500);
    }
  }

}

/*uint32_t candleTimerMs = 0;

void setup()
{
  pinMode(pinLedsCandlesString, OUTPUT);

  {
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed
    leds[0] = CRGB::Red;
    leds[1] = CRGB::Green;
    leds[2] = CRGB::Blue;
    leds[3] = CRGB(255,0,255);//CRGB::Blue;
    leds[4] = CRGB::White;
    FastLED.setBrightness(40);
    FastLED.show();
  }

  candleTimerMs = millis() + 150;
}

void loop()
{
  uint32_t _m = millis();
  if ((_m - candleTimerMs) > 0) {
    analogWrite(pinLedsCandlesString, random(150) + 105);
    //analogWrite(pinLedsCandlesString, random(170) + 85);
    //analogWrite(pinLedsCandlesString, random(120) + 135);
    candleTimerMs = _m + random(300);
  }
}*/