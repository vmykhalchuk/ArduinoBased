int pinRE = 3;
int pinDE = 4;

void setup() {
  Serial.begin(9600);
  pinMode(pinRE, OUTPUT);
  pinMode(pinDE, OUTPUT);
  digitalWrite(pinRE, HIGH);
  digitalWrite(pinDE, LOW);

  digitalWrite(pinDE, HIGH);
}

int i = 1;
void loop() {
  Serial.write(i);
  if (i == 5) i = 1; else i++;
  delay(3000);
}
