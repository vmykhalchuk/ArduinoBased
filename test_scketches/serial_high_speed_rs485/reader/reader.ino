const uint8_t pinLed = 13;

void setup() {
  pinMode(pinLed, OUTPUT);
  digitalWrite(pinLed, LOW);

  // 9600, 500000, 1000000, 2000000
  Serial.begin(9600);
  while(!Serial) {
    // some boards need to wait to ensure access to serial over USB
  }
}


uint8_t state = 0; // AbCd <200ms> AbCd <200ms>
long tmstmp;
bool errorOccurred = false;
void loop() {
  if (errorOccurred && (millis() - tmstmp) > 200) {
    errorOccurred = false;
    digitalWrite(pinLed, LOW);
  }
  if (Serial.available()) {
    char c = Serial.read();
    char expectedC;
    if (state == 0) {
      expectedC = 'A';
    } else if (state == 1) {
      expectedC = 'b';
    } else if (state == 2) {
      expectedC = 'C';
    } else if (state == 3) {
      expectedC = 'd';
    }
    if (c != expectedC) {
      tmstmp = millis();
      errorOccurred = true;
      digitalWrite(pinLed, HIGH);
    }
    state = state == 3 ? 0 : state + 1;
  }
}