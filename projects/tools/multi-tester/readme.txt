Used to test misc sensors, protocols, leds, etc
 - Sensors:
   - DS18B20
   - HTU21D
   - DHT11
 - Leds:
   - WS2811S
   - KH2441EF-8
   - TM1637
 - Misc:
   - Read special pulsed pattern to determine line No
     - In order to simultaneously test many lines (used to search wiring),
       generator board will generate four different signals to be connected to four lines.
       This function will determine if signal is coming from line 1 - line 4
   - Read multiple digital inputs and dsiplay them on KH2441EF-8
   - Read Analogue input
   - Generate PWM
   - Read Serial and display on KH2441EF-8
   - Write to Serial
