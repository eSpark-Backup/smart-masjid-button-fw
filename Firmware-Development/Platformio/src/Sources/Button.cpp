#include <Arduino.h>
#include "FunctionalInterrupt.h"
#include "Button.hpp"
#include "SM_Http.hpp"
#include "LED.hpp"

volatile int Button::counter = 0;
bool Button::resetFlag = false;
extern volatile buttonState state = UNPRESSED;
uint32_t Button::adhanStartTime = 0;
TickType_t buttonPressStartTime = 0;
TickType_t resetButtonPressStartTime = 0;
SmHttp smHttp;

volatile uint32_t lastInterruptTime = 0; // Track last interrupt time for debounce
const uint32_t debounceDelay = 200;      // Debounce delay in milliseconds
volatile bool waitingForRelease = false;

bool SMBTN_Flag = false;
static unsigned long button_check_time = 0;

bool ADHAN_FLAG = false;
bool ignore_release = false;

void Button::setup()
{
  pinMode(pin, INPUT_PULLUP);
  pinMode(RESET_GPIO, INPUT_PULLUP);
  attachInterrupt(pin, std::bind(&Button::interruptHandler, this), FALLING);
  // attachInterrupt(pin, interruptHandler, FALLING);
}

void Button::interruptHandler()
{
  uint32_t currentTime = millis();
  if (currentTime - lastInterruptTime > debounceDelay && !waitingForRelease)
  {
    lastInterruptTime = currentTime; // Update last press time
    Button::counter++;
    waitingForRelease = true;
  }
}

void Button::loop()
{
  if (waitingForRelease && digitalRead(SMBTN_GPIO) == HIGH)
  {
    waitingForRelease = false; // Button released, allow next press
  }

  if (digitalRead(RESET_GPIO) == LOW) // Check reset button
  {
    if (resetButtonPressStartTime == 0)
    {
      resetButtonPressStartTime = xTaskGetTickCount();
    }
    else if ((xTaskGetTickCount() - resetButtonPressStartTime) >= pdMS_TO_TICKS(5000))
    {
      Serial.println("Reset button pressed for 5 seconds. Restarting...");
      delay(1000); // Small delay to avoid accidental reset
      // esp_restart(); // Restart the ESP32
      Device::deleteMasjidId();
      Device::deleteWificredentials();
      ESP.restart();
    }
  }
  else
  {
    resetButtonPressStartTime = 0;
  }

  // if (digitalRead(SMBTN_GPIO) == LOW)
  // {
  //   if (buttonPressStartTime == 0)
  //   {
  //     buttonPressStartTime = xTaskGetTickCount();
  //   }
  //   else if ((xTaskGetTickCount() - buttonPressStartTime) >= pdMS_TO_TICKS(5000))
  //   {
  //     Serial.print(".");
  //     Serial.println("\nAdhan is Reset\n\n");
  //     smHttp.resetAdhan();
  //     Button::resetFlag = true;
  //     Button::counter = 0;
  //     buttonPressStartTime = 0;
  //   }
  // }
  // else
  // {
  //   buttonPressStartTime = 0;
  //   state = (buttonState)Button::counter;
  //   switch (state)
  //   {
  //   case ADHAN_START_REQUESTED:
  //     smHttp.startAdhan();
  //     Button::adhanStartTime = millis();
  //     Serial.print("ADHAN_START");
  //     state = ADHAN_STARTED;
  //     rgb.currentMode = MODE::ADHAN_STREAMING;
  //     Button::counter++;
  //     break;

  //   case ADHAN_STARTED:

  //     if ((millis() - Button::adhanStartTime) > ADHAN_MAX_DURATION)
  //     {
  //       state = ADHAN_STOP_REQUESTED;
  //       rgb.currentMode = MODE::ADHAN_RESET_MODE;
  //     }
  //     break;

  //   case ADHAN_STOP_REQUESTED:
  //     smHttp.stopAdhan();
  //     state = ADHAN_STOPPED;
  //     Serial.print("ADHAN_STOP");
  //     rgb.currentMode = MODE::ADHAN_CALLED;
  //     Button::counter = 0;
  //     break;

  //   default:
  //     break;
  //   }
  // }

  /* Update Logic with testing on ESP32 DEV Module*/
  if (digitalRead(SMBTN_GPIO) == LOW)
  {
    if (button_check_time == 0)
    {
      button_check_time = millis();
    }

    if (millis() - button_check_time > 100) // minor delay for button debounce
    {
      ADHAN_FLAG = true;

      if (buttonPressStartTime == 0)
      {
        buttonPressStartTime = xTaskGetTickCount();
      }
      else if ((xTaskGetTickCount() - buttonPressStartTime) >= pdMS_TO_TICKS(5000))
      {
        Serial.print(".");
        Serial.println("\nAdhan is Reset\n");
        smHttp.resetAdhan();
        rgb.currentMode = MODE::ADHAN_RESET_MODE;
        Button::resetFlag = true;
        Button::counter = 0;
        buttonPressStartTime = 0;
        button_check_time = 0;
        ADHAN_FLAG = false;
        ignore_release = true;
      }
    }
  }
  else
  {
    button_check_time = 0;
    buttonPressStartTime = 0;
    if (ignore_release)
    {
      ignore_release = false;
      ADHAN_FLAG = false;
    }
    else if (ADHAN_FLAG)
    {
      state = (buttonState)Button::counter;
      switch (state)
      {
      case ADHAN_START_REQUESTED:
        smHttp.startAdhan();
        Button::adhanStartTime = millis();
        Serial.print("\nADHAN_START");
        rgb.currentMode = MODE::ADHAN_STREAMING;
        Button::counter = 1;
        ADHAN_FLAG = false;
        break;
      case ADHAN_STARTED:
        smHttp.stopAdhan();
        Button::counter = 0;
        // state = ADHAN_STOPPED;
        Serial.print("\nADHAN_STOP\n");
        rgb.currentMode = MODE::ADHAN_CALLED;
        ADHAN_FLAG = false;
        ignore_release = true;
        break;
      default:
        break;
      }
    }
  }

  if (Button::counter == ADHAN_STARTED)
  {
    if ((millis() - Button::adhanStartTime) > ADHAN_MAX_DURATION)
    {
      // state = ADHAN_STOP_REQUESTED;
      Button::counter = 2;
      Serial.print("\nADHAN_STOP_REQUESTED");
      Button::adhanStartTime = 0;
      rgb.currentMode = MODE::ADHAN_CALLED;
    }
  }
  else if (Button::counter == ADHAN_STOP_REQUESTED)
  {
    smHttp.stopAdhan();
    Button::counter = 0;
    // state = ADHAN_STOPPED;
    Serial.print("\n--------ADHAN_STOP\n");
    rgb.currentMode = MODE::ADHAN_CALLED;
  }

  vTaskDelay(pdMS_TO_TICKS(100));
}
