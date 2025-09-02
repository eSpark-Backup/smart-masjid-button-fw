#include <Arduino.h>
#include "FunctionalInterrupt.h"
#include "Button.hpp"
#include "BLE.hpp"
#include "SM_Http.hpp"
#include <WiFi.h>
#include "Sources/WifiProvisioning.hpp"
#include "esp_wifi.h"


WifiProvisioning wifiProvis;
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

void check_reset_button(){
  if (digitalRead(RESET_GPIO) == LOW) // Check reset button
    {
      if (resetButtonPressStartTime == 0)
      {
      resetButtonPressStartTime = xTaskGetTickCount();
      }
      else if ((xTaskGetTickCount() - resetButtonPressStartTime) >= pdMS_TO_TICKS(5000))
      {
        Serial.println("Reset button pressed for 5 seconds. Switching to BLE Mode...");
        delay(1000); // Small delay to avoid accidental reset
        // Device::deleteMasjidId();
        Device::deleteWificredentials();

        // New Feature added for Turn ON BLE Mode after pressing Reset Button for 5 seconds
        WiFi.disconnect();
        WiFi.mode(WIFI_OFF);
        delay(500);

        // switch to Bluetooth mode
        Serial.println("Switching to Bluetooth mode");
        if(!BLE::isStarted()){
          Serial.println("BLE is complete OFF");
          BLE::start();
        }
        Serial.println("BLE initialized");
        while (WiFi.status() != WL_CONNECTED)
        {
          Serial.print(".");
          delay(100);
        }
        BLE::stop();
        // ESP.restart();
      }
      Serial.println("Reset button pressed");
    }
    else
    {
      resetButtonPressStartTime = 0;
    }
}
void Button::loop()
{
  if (waitingForRelease && digitalRead(SMBTN_GPIO) == HIGH)
  {
    waitingForRelease = false; // Button released, allow next press
  }
  check_reset_button();

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
