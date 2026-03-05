#define DHT11_PIN 7

void setup() {
  Serial.begin(9600);
}

void loop() {
  byte data[5] = {0, 0, 0, 0, 0};
  
  // 1. START SIGNAL
  pinMode(DHT11_PIN, OUTPUT);
  digitalWrite(DHT11_PIN, LOW);
  delay(18);         // Host pulls low for at least 18ms
  digitalWrite(DHT11_PIN, HIGH);
  delayMicroseconds(30); // !!! another AI reincarnation uses 40
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
    Serial.print("Humidity: ");
    Serial.print(data[0]);
    Serial.print("%  Temperature: ");
    Serial.print(data[2]);
    Serial.println("C");
  } else {
    Serial.println("Checksum Error!");
  }

  delay(2000); // DHT11 needs 2 seconds between reads
}