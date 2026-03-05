#define DS18B20_PIN 4

void setup() {
  Serial.begin(9600);
  setResolution(0x3F); // 10-bit to reduce conversion time
}

void loop() {
  float temperature = readTemperature();
  if (temperature != -1000.0) {
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");
  } else {
    Serial.println("Sensor not found.");
  }
  delay(1000); 
}

// --- 1-Wire Low Level Functions ---

bool ds_reset() {
  noInterrupts();
  pinMode(DS18B20_PIN, OUTPUT);
  digitalWrite(DS18B20_PIN, LOW);
  delayMicroseconds(480); // Reset pulse
  pinMode(DS18B20_PIN, INPUT);
  delayMicroseconds(70);  // Wait for presence pulse
  bool presence = !digitalRead(DS18B20_PIN);
  interrupts();
  delayMicroseconds(410); // Finish the time slot
  return presence;
}

void ds_write_bit(uint8_t bit) {
  pinMode(DS18B20_PIN, OUTPUT);
  digitalWrite(DS18B20_PIN, LOW);
  if (bit) {
    delayMicroseconds(10); 
    pinMode(DS18B20_PIN, INPUT);
    delayMicroseconds(55);
  } else {
    delayMicroseconds(65);
    pinMode(DS18B20_PIN, INPUT);
    delayMicroseconds(5);
  }
}

void ds_write_byte(uint8_t data) {
  noInterrupts();
  for (uint8_t i = 0; i < 8; i++) {
    ds_write_bit(data & 0x01);
    data >>= 1;
  }
  interrupts();
}

uint8_t ds_read_bit() {
  uint8_t bit = 0;
  pinMode(DS18B20_PIN, OUTPUT);
  digitalWrite(DS18B20_PIN, LOW);
  delayMicroseconds(3);
  pinMode(DS18B20_PIN, INPUT);
  delayMicroseconds(10);
  if (digitalRead(DS18B20_PIN)) bit = 1;
  delayMicroseconds(50);
  return bit;
}

uint8_t ds_read_byte() {
  noInterrupts();
  uint8_t data = 0;
  for (uint8_t i = 0; i < 8; i++) {
    if (ds_read_bit()) data |= (1 << i);
  }
  interrupts();
  return data;
}

// --- High Level Temperature Conversion ---

float readTemperature() {
  if (!ds_reset()) return -1000.0;

  ds_write_byte(0xCC); // Skip ROM (only works with one sensor)
  ds_write_byte(0x44); // Start Conversion

  // Wait for conversion (750ms for 12-bit)
  // While converting, the sensor pulls the bus low. 
  // We'll just wait for it to return to high.
  while (!ds_read_bit()); 

  if (!ds_reset()) return -1000.0;
  ds_write_byte(0xCC); // Skip ROM
  ds_write_byte(0xBE); // Read Scratchpad

  uint8_t lowByte = ds_read_byte();
  uint8_t highByte = ds_read_byte();

  // Combine bytes into a 16-bit signed integer
  int16_t raw = (highByte << 8) | lowByte;
  
  // Convert to Celsius: DS18B20 uses 0.0625°C per LSB
  return (float)raw * 0.0625;
}


/*
  The configuration byte follows this pattern: 0 R1 R0 1 1 1 1 1.
  9-bit: 0x1F (00011111)
  10-bit: 0x3F (00111111)
  11-bit: 0x5F (01011111)
  12-bit: 0x7F (01111111)
*/
void setResolution(uint8_t hexValue) {
  if (ds_reset()) {
    ds_write_byte(0xCC); // Skip ROM (talk to all/single sensor)
    ds_write_byte(0x4E); // Write Scratchpad command
    
    ds_write_byte(0x00); // TH Alarm (unused, but must be written)
    ds_write_byte(0x00); // TL Alarm (unused, but must be written)
    ds_write_byte(hexValue); // The Configuration Byte (Resolution)
    
    // Optional: Save to EEPROM so it survives power loss
    /*
    ds_reset();
    ds_write_byte(0xCC);
    ds_write_byte(0x48); // Copy Scratchpad to EEPROM
    delay(10);           // Wait for EEPROM write
    */
  }
}
