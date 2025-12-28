int led1 = 10;

void setup()
{
  pinMode(led1, OUTPUT);
}

void loop()
{
  analogWrite(led1, random(120) + 135);
  delay(random(150));
}

