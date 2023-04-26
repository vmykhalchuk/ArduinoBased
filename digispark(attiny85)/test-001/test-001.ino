const int pinLed = 1;

void setup() {
  pinMode(5, OUTPUT);
  digitalWrite(5, LOW);

  pinMode(pinLed, OUTPUT);
}

void loop() {
  digitalWrite(pinLed, HIGH);
  delay(200);
  digitalWrite(pinLed, LOW);
  delay(200);
}