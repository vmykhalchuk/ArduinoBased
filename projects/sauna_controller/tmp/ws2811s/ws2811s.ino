/*
 * WS2811 "No Library" Bit-Banging
 * Target: Arduino Nano (16MHz ATmega328P)
 * Pin: Digital 4 (PD4)
 */

#define LED_COUNT 3
#define LED_PIN   3 // Port D, Pin 3

// Structure to hold RGB data
struct RGB {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

RGB leds[LED_COUNT];

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
}

void loop() {
  // Create a simple animation: cycle colors
  for (int i = 0; i < LED_COUNT; i++) {
    leds[i] = {200, 0, 0}; // Red
  }
  show();
  delay(500);

  for (int i = 0; i < LED_COUNT; i++) {
    leds[i] = {0, 200, 0}; // Green
  }
  show();
  delay(500);

  for (int i = 0; i < LED_COUNT; i++) {
    leds[i] = {0, 0, 200}; // Blue
  }
  show();
  delay(500);
}

// The core "Bit-Bang" function
void show() {
  volatile uint8_t *port = &PORTD; // Pointer to Port D
  uint8_t pinMask = (1 << LED_PIN); 
  
  // Disable interrupts so timing isn't interrupted by background tasks
  cli(); 

  for (int i = 0; i < LED_COUNT; i++) {
    sendByte(leds[i].r, port, pinMask);
    sendByte(leds[i].g, port, pinMask);
    sendByte(leds[i].b, port, pinMask);
  }

  sei(); // Re-enable interrupts
  delayMicroseconds(50); // Reset pulse
}

// Inline function to handle high-speed bit transmission
void sendByte(uint8_t byte, volatile uint8_t *port, uint8_t mask) {
  for (int bit = 7; bit >= 0; bit--) {
    if (byte & (1 << bit)) {
      // Send a '1' bit
      *port |= mask;   // Drive HIGH
      asm volatile("nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t"); 
      *port &= ~mask;  // Drive LOW
      asm volatile("nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t");
    } else {
      // Send a '0' bit
      *port |= mask;   // Drive HIGH
      asm volatile("nop\n\t" "nop\n\t");
      *port &= ~mask;  // Drive LOW
      asm volatile("nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t");
    }
  }
}
