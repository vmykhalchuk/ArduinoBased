
Used to test misc sensors, protocols, leds, etc

Programs:
 P00: DS18B20 Temp Sensor
 P01: HTU21D Temp & Hum Sensor
 P02: DHT11 Temp & Hum Sensor
 P03: Analog Read
 P04: Multiple digital inputs
 
 P10: WS2811S - out to 10 Leds
 P11: TM1637
 P12: Generate PWM

 P20: Read special pulsed pattern with ID+CRC
 P21: Read errors (specialized for my projects)
 P22: Read Serial
 P23: Write Serial

 P99: Select auto-start program (stores in EEPROM)

------------------------------------------------

 P20: Read special pulsed pattern with ID+CRC

This program reads special signal to determine which line ID is input receiving.
Another board is transmitting unique IDs with CRC over many lines.
When input D? is connected to any of that line, it will read ID+CRC transfered
and display on KH2441EF screen

DEV NOTES:
 - use optoisolation for transmitter and receiver
 - send length-encoded pulses, however slow ones to overcome long line capacitance
 - send CRC/inverted ID to validate connection
 - when receiving any data (data validation fails) - display blinking '---'
     to indicate that any signal is being received

------------------------------------------------

 P21: Read errors (specialized for my projects)

This program will read errors stream from single wire. This is my protocol
to debug programs. Similar to OneWire.

------------------------------------------------
