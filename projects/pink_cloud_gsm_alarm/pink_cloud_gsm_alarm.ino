const uint8_t pinLedsRGBString = 2;//A1;
const uint8_t pinIRSensorIn = 3;
const uint8_t pinDoorSmartHouseIn = 4;
const uint8_t pinDoorVitalnaIn = 5;


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
  Serial.begin(9600);
  while(!Serial) {
    // some boards need to wait to ensure access to serial over USB
  }

  pinMode(pinIRSensorIn, INPUT_PULLUP);
  pinMode(pinDoorSmartHouseIn, INPUT_PULLUP);
  pinMode(pinDoorVitalnaIn, INPUT_PULLUP);

  Serial.write("AT\r\n");

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed
  leds[0] = CRGB::Red;
  leds[1] = CRGB::Green; //CRGB::Blue;//CRGB(255,0,255);//CRGB::White;
  FastLED.setBrightness(40);
  FastLED.show();

  delay(5000);
  Serial.write("ATD+380979635113;\r\n");

  leds[1] = CRGB::Red;
  FastLED.show();

  delay(10000);

  leds[0] = CRGB::Black;
  leds[1] = CRGB::Black;
  FastLED.show();
}

// GSM commands:
//   AT
//   ATD+38.....;
//   AT+CLCC
//   AT+CHUP
//   ATH

// receives:
//   RING
//
//   +CLIP: "+380975411368",145,"",0,"",0
//
//   NO CARRIER

void loop() {
  if (Serial.available()) Serial.read();
  if (digitalRead(pinIRSensorIn) == LOW || digitalRead(pinDoorSmartHouseIn) == HIGH || digitalRead(pinDoorVitalnaIn) == HIGH) {
    leds[0] = CRGB::Red;
    FastLED.show();
    // ALARMA!!!!
    Serial.write("ATD+380979635113;\r\n");
    delay(10000);
    leds[0] = CRGB::Black;
    FastLED.show();
  }
}
