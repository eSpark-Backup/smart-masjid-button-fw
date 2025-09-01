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
MODE rgb_last_mode;

volatile uint32_t lastInterruptTime = 0;  // Track last interrupt time for debounce
const uint32_t debounceDelay = 200;  // Debounce delay in milliseconds
volatile bool waitingForRelease = false;
int32_t expiration_time = 0, stop_requested_time = 0;
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
    lastInterruptTime = currentTime;  // Update last press time
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
      Serial.print("ADHAN_START\n");
      rgb_last_mode = rgb.currentMode;
      state = ADHAN_STARTED;
      rgb.currentMode = MODE::ADHAN_STREAMING;
      Button::counter++;
      break;

    case ADHAN_STARTED:
      
      if ((millis() - Button::adhanStartTime) > ADHAN_MAX_DURATION)
      {
        state = ADHAN_STOP_REQUESTED;
        // rgb.currentMode = MODE::ADHAN_RESET_MODE;
      }
      break;

    case ADHAN_STOP_REQUESTED:
      
      if(stop_requested_time == 0){
        // expiration_time = 1800; // For testing purposes only
        smHttp.stopAdhan(&expiration_time);
        stop_requested_time = millis();
        rgb.currentMode = MODE::ADHAN_CALLED;
        Serial.print("ADHAN_STOP_REQUESTED\n");
      }
      if(millis()- stop_requested_time >= expiration_time){
        state = ADHAN_STOPPED;
        Serial.print("ADHAN_STOP\n");
        rgb.currentMode = rgb_last_mode;
        Button::counter = 0;
        stop_requested_time = 0;
      }
      break;

    default:
      break;
    }
  }

  vTaskDelay(pdMS_TO_TICKS(100));
}
