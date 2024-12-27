#include <Arduino.h>
#include "FunctionalInterrupt.h"
#include "Button.hpp"
#include "SM_Http.hpp"

volatile int Button::counter = 0;
bool Button::resetFlag = false;
extern volatile buttonState state = UNPRESSED;
uint32_t Button::adhanStartTime = 0;
TickType_t buttonPressStartTime = 0;
SmHttp smHttp;

void Button::setup()
{
  pinMode(pin, INPUT_PULLUP);
  attachInterrupt(pin, std::bind(&Button::interruptHandler, this), FALLING);
    // attachInterrupt(pin, interruptHandler, FALLING);
}

void Button::interruptHandler()
{
  Button::counter++;
}

void Button::loop()
{
  if (digitalRead(SMBTN_GPIO) == LOW)
  {
    if (buttonPressStartTime == 0)
    {
      buttonPressStartTime = xTaskGetTickCount();
    }
    else if ((xTaskGetTickCount() - buttonPressStartTime) >= pdMS_TO_TICKS(5000))
    {
      Serial.print(".");
      Serial.println("\nAdhan is Reset\n\n");
      smHttp.resetAdhan();
      Button::resetFlag = true;
      Button::counter = 0;
      buttonPressStartTime = 0;
    }
  }
  else
  {
    buttonPressStartTime = 0;
    state = (buttonState)Button::counter;
    switch (state)
    {
      case ADHAN_START_REQUESTED:
        smHttp.startAdhan();
        Button::adhanStartTime = millis();
        Serial.print("ADHAN_START");
        state = ADHAN_STARTED;
        Button::counter++;
        break;

      case ADHAN_STARTED:
        if ((millis() - Button::adhanStartTime) > ADHAN_MAX_DURATION)
        {
          state = ADHAN_STOP_REQUESTED;
        }
        break;

      case ADHAN_STOP_REQUESTED:
        smHttp.stopAdhan();
        state = ADHAN_STOPPED;
        Serial.print("ADHAN_STOP");
        Button::counter = 0;
        break;

      default:
        break;
    }
  }

  vTaskDelay(pdMS_TO_TICKS(100));
}
