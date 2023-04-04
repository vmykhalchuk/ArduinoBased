void setup() {
  // 9600, 500000, 1000000, 2000000
  Serial.begin(9600);
  while(!Serial) {
    // some boards need to wait to ensure access to serial over USB
  }
}

void loop() {
  Serial.print("SASASASA");
  for (uint8_t i = 0; i < ('z'-'a'); i++) {
    Serial.print((char)('a'+i));
  }
}