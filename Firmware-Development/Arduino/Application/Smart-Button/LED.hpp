#pragma once

#include<Arduino.h>

class LED
{
  private:
    int ledPin1, ledPin2, ledPin3;

  public:
    LED(int pin1, int pin2, int pin3);
    void setup();
    void set(int pin);
    void reset(int pin);
};
