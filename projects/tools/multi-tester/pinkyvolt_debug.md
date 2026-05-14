
## Technical details

### Communication protocol

Transmitter and Receiver are connected via single line.

Transmitter utilizes D5 pin (in three state mode) that directly connects to line.

Receiver connects D5 (in input mode) directly to line, D6 to output of comparator that compares line voltage to 0.12V reference.
Additionally Receiver connects D7 (as output) via 2.7k resistor to LINE. When D7 is set to OUTPUT+LOW it pulls Line Down. And is disengaged when D7 is set to INPUT.

#### Starting sequence:
  - Transmitter
    - sets D5 to Input with internal pullup enabled
    - waits for line to become LOW (if HIGH - no Receiver connected, or connection issue)
  - Receiver
    - makes sure D7 is LOW (2k resistor pull-down is disabled)
    - waits for line to become M (if LOW - no Transmitter connected, or connection issue)

#### Handshake:
  - Receiver
    - pulls line down via 2.7k resistor (set D7 to HIGH), waits 1ms
    - waits for line to become M (if it is LOW - no Transmitter connected, or connection issue)

#### Protocol:

**Notes:** Internal or external comparator (LM393 or similar) will monitor line.
         Line length <= 2cm (comparator will be placed near connector and powered from Transmitter, then will send data over longer line to Receiver)


#### Hardware implementation

** Connect Transmitter via 200 Ohm resistor to avoid short circuit **

Select Comparator for task

|   IC    |   time  | offset | Ib  | Umin |
|:-------:|:-------:|:------:|:---:|:----:|
| LM393N  |  1.5uS  |  5mV   | 25nA|      |
| MCP6541 |   4uS   |  7mV   | 1pA |      |
| LM311DR |  115nS  | 7.5mV  |250nA| 3.5V |
| LM311DR |  200nS  | 7.5mV  |250nA|  5V  |
| LM311P  |  115nS  | 7.5mV  |250nA| 3.5V |

**LM393N circuit with hysteresis 0.02V**
  - R1=47k, R2=1k, RH=240k, Rp=10k => 90-110mV
  - R1=33k => 140-160mV
  - Input goes to (-)
  - R1 between VCC and (+)
  - R2 between GND and (+)
  - RH between (+) and Output
  - Rp between Output and VCC


#### Pull-down specification

|     |  2k / 5V  | 2k / 4.5V | 1.9k / 4.5V |
|:---:|:---------:|:---------:|:-----------:|
| 55k | 0.175     | 0.158     | 0.150       |
| 50k | 0.192     |           |             |
| 30k | 0.313     |           |             |
| 20k | 0.455     |           |             |
| 15k | 0.588     |           | 0.506       |

*V=5V*2k/(55k+2k)*

**Alternative (2.7k):**

|     |  2.7k/5V  | 2.7k/4.5V |  2.6k/4.5V  |
|:---:|:---------:|:---------:|:-----------:|
| 55k | 0.234     | 0.211     | 0.196       |
| 50k | 0.256     |           |             |
| 30k | 0.413     |           |             |
| 20k | 0.595     |           |             |
| 15k | 0.763     |           | 0.636       |

*V=5V*2.7k/(55k+2.7k)*

#### Design improvement - Active pull-up at start

##### Idea 1 : Suggestion 1
Currently when we connect Receiver to Transmitter - connection will present jittering on line, causing protocol failures.
To prevent this - we can drive Line on Receiver side to HIGH via 200Ohm resistor.
And after a while - when connection established - disconnect that pull-up.

Cons: Receiver should be commanded by user to start communication.
   This is tradeoff - tolerate communication failure at start and let timeouts restart it, however user doesn't have to do anything
                    - or force user to start communication after physical connection is done
Cons: User might forget to disable communication and break physical connection and re-connect again - causing initial issue we try to solve here.

##### Idea 1 : Suggestion 2 (best if this resistor will not affect Comparator)

We can instead pull LOW via big resistor (400K), and monitor input - when it comes HIGH (due to PU on TX Side - after delay we can start communication)
Also Pull down should be very close to connector and should use comparator to send strong signal to MCU.

##### Idea 1 : Suggestion 3

Add Debouncing stage into TX Handshake FSM (debounce when Handshake starts)


#### Electrical considerations

**Ask AI to solve this:**
We have two atmega328 mcus connected via single line 10cm long (Source D4 to Target D5).
D4 is Input no internal pullup. D5 in input no internal pullup.
At start Target pulls line down via 2.7k resistor (via its' D7 pin).
After a while Source activates internal pull-up and line settles.
Then after another while Source drive line High.
And then after line settles drives line Low.

Describe in details that final transition (how long it takes, why, what voltage transients are)

** Also do it for other transitions **
