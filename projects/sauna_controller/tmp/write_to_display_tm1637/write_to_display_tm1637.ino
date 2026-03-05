// Define pins
#define CLK 2
#define DIO 3

// Seven-segment patterns for 0-9
const uint8_t segmentMap[] = {
  0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f
};

void setup() {
  pinMode(CLK, OUTPUT);
  pinMode(DIO, OUTPUT);
}

void loop() {
  displayInteger(1234, true);
  delay(1000);
  displayInteger(1234, false);
  delay(1000);
}

// --- Low Level Communication ---

void start() {
  digitalWrite(CLK, HIGH);
  digitalWrite(DIO, HIGH);
  delayMicroseconds(5); // can be 2 (try it first), same for all 5us
  digitalWrite(DIO, LOW);
}

void stop() {
  digitalWrite(CLK, LOW);
  digitalWrite(DIO, LOW);
  delayMicroseconds(5);
  digitalWrite(CLK, HIGH);
  digitalWrite(DIO, HIGH);
  delayMicroseconds(5);
}

bool writeByte(uint8_t b) {
  for (uint8_t i = 0; i < 8; i++) {
    digitalWrite(CLK, LOW);
    digitalWrite(DIO, (b & 0x01) ? HIGH : LOW); // Send LSB first
    b >>= 1;
    delayMicroseconds(5);
    digitalWrite(CLK, HIGH);
    delayMicroseconds(5);
  }
  
  // Wait for ACK
  digitalWrite(CLK, LOW);
  pinMode(DIO, INPUT);
  delayMicroseconds(5);
  digitalWrite(CLK, HIGH);
  delayMicroseconds(5);
  bool ack = digitalRead(DIO) == 0;
  pinMode(DIO, OUTPUT);
  return ack;
}

// --- High Level Functions ---

void displayInteger(int num, bool showColon) {
  uint8_t digits[4];
  digits[0] = segmentMap[(num / 1000) % 10];
  digits[1] = segmentMap[(num / 100) % 10];
  digits[2] = segmentMap[(num / 10) % 10];
  digits[3] = segmentMap[num % 10];

  // apply colon to second digit
  if (showColon) {
    digits[1] |= 0x80;
  }

  // 1. Data Command: Setting automatic address adding
  start();
  writeByte(0x40); 
  stop();

  // 2. Address Command: Set start address to 0xC0 (first digit)
  start();
  writeByte(0xC0);
  for (int i = 0; i < 4; i++) {
    writeByte(digits[i]);
  }
  stop();

  // 3. Display Control: Turn on display, set brightness (0x88 is on, 0x8f is max)
  start();
  writeByte(0x88 + 3); // 0x88 is "ON", +7 is max brightness 
  stop();
}