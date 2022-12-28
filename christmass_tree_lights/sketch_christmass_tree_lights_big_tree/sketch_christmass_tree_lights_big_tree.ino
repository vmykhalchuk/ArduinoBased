// This sketch is used for three lights combined together (see stored device on main Christmass tree)
// channels looks like are mismatched, so below port numbers are corrected accordingly

int led1 = 10;//9;
int led2 = 9;//10;
int led3 = 11;//11

void setup()
{
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
}

void loop()
{
  // running light effect
  for (int k = 0; k < 50; k++) {
    for (int i = 0; i < 3; i++) {
      if (i == 0) {
        fadeFromTo(led1, led2);
      } else if (i == 1) {
        fadeFromTo(led2, led3);
      } else if (i == 2) {
        fadeFromTo(led3, led1);
      }
      delay(50);
    }
  }
  for (int k = 0; k < 50; k++) {
    for (int i = 0; i < 3; i++) {
      if (i == 0) {
        fadeFromTo(led1, led3);
      } else if (i == 1) {
        fadeFromTo(led3, led2);
      } else if (i == 2) {
        fadeFromTo(led2, led1);
      }
      delay(50);
    }
  }

  // candle like effect
  for (int i = 0; i < 150; i++) {
    analogWrite(led1, random(150) + 105);
    analogWrite(led2, random(170) + 85);
    analogWrite(led3, random(120) + 135);
    delay(random(150));
  }

}

void fadeFromTo(int ledA, int ledB) {
  for (int i = 0; i <= 250; i+=10) {
    analogWrite(ledA, i);
    delay(1);
  }
  for (int i = 0; i <= 250; i+=10) {
    analogWrite(ledB, 250-i);
    delay(1);
  }
}
