
Used to test misc sensors, protocols, components, etc

When tool starts, it displays for 2 seconds: "Pxx" - where xx - number of program to start.
If user wants another program to start with - long-press during this 2 seconds window in order
to go into select program mode.

In select program mode, single press increments program number. Double click increases major digit.

Long press to confirm program selected.

Programs:
+P00: DS18B20 Temp Sensor
+P01: HTU21D/SHT2x Temp & Hum Sensor
-P02: DHT11 Temp & Hum Sensor
+P03: Analog Read
-P04: High impedance digital inputs
 
-P10: WS2811S - out to 10 Leds
-P11: TM1637
-P12: Generate PWM
-P13: Read first pulse width

-P20: Pinkyvolt Debug Reader (specialized for my projects)
-P21: Read special pulsed pattern with ID+CRC
-P22: Read Serial
-P23: Write Serial
-P24: Scan Serial (try different bits and parity till some valid content is produced)

+P99: Select auto-start program (stores in EEPROM)

------------------------------------------------

 P13: Read first pulse width

Note: Currently impossible to make since Digital Pin 8 (PB0) is required.

Lets user read incoming pulse width (with max resolution 0.0625uS).
User should be able to change resolution.
When user presses Sel button - current line state is taken as base. Whenever line changes
to opposite state - timer starts, when back - timer stops and value displayed.
Another pulse width counting starts after user presses Sel again.

------------------------------------------------

 P20: Pinkyvolt Debug Reader (specialized for my projects)

This program will read bytes message streamed over single wire via Pinkyvolt Debug protocol.
Data received will be displayed as Hex string scrolled over display.
After frame is received - program displays message till user presses button to read next frame.

------------------------------------------------

 P21: Read special pulsed pattern with ID+CRC

This program reads special signal to determine which line ID is input receiving.
Another board is transmitting unique IDs with CRC over many lines.
When input D? is connected to any of that line, it will read ID+CRC transfered
and display on KH2441EF screen

DEV NOTES:
 - use optoisolation for transmitter and receiver
 - send length-encoded pulses, however slow ones to overcome long line capacitance
 - send CRC/inverted ID to validate connection
 - when receiving any data (data validation fails) - display blinking '---'
     to indicate that some signal is still being received

------------------------------------------------

 P99: Select auto-start program (stores in EEPROM)

In this mode user selects program to be used as default one. After MCU restarts - it will be
auto-executed.
------------------------------------------------


  -=-=-=--=-=- ERRORs -=-=-=-=-=-=--=
  
EEPROM Failures:
  When screen transitions between three values:
    - EPr
    - Err
    - Exx, xx - two digits error code
      - 01 - 04     - System error: Config error
      - 05 - 09,11  - System error: Alg error
      - 10          - EEPROM Read failure - Bad CRC
      - 12-16       - EEPROM Corrupted
      - 17          - System error: Alg error
      