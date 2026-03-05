// Define pins
#define CLK 2
#define DIO 3

#define DHT11_PIN 7

// Seven-segment patterns for 0-9
const uint8_t segmentMap[] = {
  0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f
};

void setup() {
  pinMode(CLK, OUTPUT);
  pinMode(DIO, OUTPUT);
}

void loop() {
  int temp = readTempDHT11();
  for (int i = 0; i < 3; i++) {
    displayInteger(2000 + temp, true);
    delay(1000);
    displayInteger(3000 + temp, false);
    delay(1000);
  }
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

// --- DHT11 functionality ---
int readTempDHT11() {
  byte data[5] = {0, 0, 0, 0, 0};
  
  // 1. START SIGNAL
  pinMode(DHT11_PIN, OUTPUT);
  digitalWrite(DHT11_PIN, LOW);
  delay(18);         // Host pulls low for at least 18ms
  digitalWrite(DHT11_PIN, HIGH);
  delayMicroseconds(40); // !!! another AI reincarnation uses 30
  pinMode(DHT11_PIN, INPUT);

  // 2. ACKNOWLEDGEMENT
  // Wait for sensor to pull low (80us) and then high (80us)
  while(digitalRead(DHT11_PIN) == HIGH); 
  while(digitalRead(DHT11_PIN) == LOW);  
  while(digitalRead(DHT11_PIN) == HIGH); 

  // 3. READ 40 BITS
  for (int i = 0; i < 40; i++) {
    while(digitalRead(DHT11_PIN) == LOW); // Wait for the 50us LOW pulse to end
    
    unsigned long startTime = micros();
    while(digitalRead(DHT11_PIN) == HIGH); // Measure how long HIGH lasts
    
    if ((micros() - startTime) > 40) {
      // If HIGH lasts longer than 40us, it's a '1' // again maybe 30!
      data[i / 8] <<= 1;
      data[i / 8] |= 1;
    } else {
      // Otherwise, it's a '0'
      data[i / 8] <<= 1;
    }
  }

  // 4. PARSE AND CHECK CHECKSUM
  // Byte 0: Humidity Integer | Byte 2: Temp Integer | Byte 4: Checksum
  if (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
    return data[2];
    //Serial.print("Humidity: ");
    //Serial.print(data[0]);
    //Serial.print("%  Temperature: ");
    //Serial.print(data[2]);
    //Serial.println("C");
  } else {
    return -1000;
    //Serial.println("Checksum Error!");
  }

  //delay(2000); // DHT11 needs 2 seconds between reads
}