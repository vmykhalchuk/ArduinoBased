const uint8_t pinLed = 13;

void setup() {
  pinMode(pinLed, OUTPUT);
  digitalWrite(pinLed, HIGH);

  // 9600, 500000, 1000000, 2000000
  Serial.begin(9600);
  while(!Serial) {
    // some boards need to wait to ensure access to serial over USB
  }
}

void loop() {
}