int pinRE = 3;
int pinDE = 4;


void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT); // Use the onboard LED for testing
  pinMode(pinRE, OUTPUT);
  pinMode(pinDE, OUTPUT);
  digitalWrite(pinRE, HIGH);
  digitalWrite(pinDE, LOW);

  digitalWrite(pinRE, LOW);
}

void loop() {
  if (Serial.available() > 0) {
    byte incomingValue = Serial.read();
    if (incomingValue >= 1 && incomingValue <= 5) {
      handleValue(incomingValue);
    }
  }
}

void handleValue(byte val) {
  for(int i = 0; i < val; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);
  }
}

void loop_read_string() {
  String inputString = "";
  inputString.reserve(200);
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      Serial.println(inputString); //???
      inputString = "";
    }
  }
}
