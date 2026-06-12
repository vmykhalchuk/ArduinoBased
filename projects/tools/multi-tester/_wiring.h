   Function       - Pins                      - Programm

- Select Button   - GND,D11
- Exit Button     - GND,D12
- KH2441EF        - D5,D6,D7,D8,D9,D10

- DS18B20         - VCC, D4, GND              - P00
                    (Three pin connector)
                    (4.7k resistor VCC-D4)
- HTU21D/SHT2x    - 3V3,GND,A4(SDA),A5(SCL)   - P01
                    (Four pin connector)
- DHT11           - Same as DS18B20           - P02
                    (only 10k resistor)
- Analog Input    - GND,A0                    - P03
- High impedance  - D0,D1,D2,D3,D4,D11,D12,   - P04
   digital inputs   D13,A0-A5

- WS2811S/WS2812B - GND,VCC,D4                - P10
- TM1637          - GND,VCC,D2,D3             - P11
- Generate PWM    - GND,D3                    - P12
   
- Pinkyvolt×Debug - GND,D2,D3                 - P20
                    2.2k resistor between D2
                    and D3 (after connected)
- Serial Input    - GND,D0                    - P22,P24
- Serial Output   - GND,D1                    - P23


Connector #1:
  GND,VCC,D0,D2,D3,D4
  Used to connect:
    - DS18B20/DHT11 connector
    - High impedance digital inputs
    - WS2811S/WS2812B
    - TM1637
    - PWM output
    - Pinkyvolt×Debug interface
    - Serial input
    
Connector #2:
  GND,VCC,3V3,A4,A5