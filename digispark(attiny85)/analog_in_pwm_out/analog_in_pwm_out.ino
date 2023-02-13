// http://digistump.com/wiki/digispark/tutorials/basics#analog_writeaka_pwm
int pinPwmOut = 0;
// http://digistump.com/wiki/digispark/tutorials/basics#analog_read
int pinAnalogIn = 2; // 2->P4; 1->P2; 2->P3;0->P5

void setup() {
  pinMode(pinPwmOut, OUTPUT);
}

void loop() {
  int v = analogRead(pinAnalogIn);
  int r = map(v, 0, 1023, 0, 255);
  analogWrite(pinPwmOut, r);
}