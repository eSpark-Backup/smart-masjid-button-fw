
#include"LED.hpp"

LED::LED(int pin1, int pin2, int pin3)
{
  ledPin1 = pin1;
  ledPin2 = pin2;
  ledPin3 = pin3;
}

void LED::setup()
{
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin3, OUTPUT);
}

void LED::set(int pin)
{
 digitalWrite(pin, HIGH);
}

void LED::reset(int pin)
{
 digitalWrite(pin, LOW);
}
