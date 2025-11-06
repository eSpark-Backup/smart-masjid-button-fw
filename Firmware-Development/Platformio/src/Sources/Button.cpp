#include <Arduino.h>
#include "FunctionalInterrupt.h"
#include "Button.hpp"
#include "BLE.hpp"
#include "SM_Http.hpp"
#include <WiFi.h>
#include "Sources/WifiProvisioning.hpp"
#include "esp_wifi.h"
#include "LED.hpp"

WifiProvisioning wifiProvis;
volatile int Button::counter = 0;
bool Button::resetFlag = false;
extern volatile buttonState state = UNPRESSED;
uint32_t Button::adhanStartTime = 0;
TickType_t buttonPressStartTime = 0;
TickType_t resetButtonPressStartTime = 0;
SmHttp smHttp;
bool BLE_mode = false;

volatile uint32_t lastInterruptTime = 0; // Track last interrupt time for debounce
const uint32_t debounceDelay = 200;      // Debounce delay in milliseconds
volatile bool waitingForRelease = false;

bool SMBTN_Flag = false;
static unsigned long button_check_time = 0;

bool ADHAN_FLAG = false;
bool ignore_release = false;

// MODE rgb_last_mode = RGB_OFF;

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
    lastInterruptTime = currentTime; // Update last press time
    if(Button::counter == 0){
      Button::counter++;
    }
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
        Device::deleteMasjidId();
        Device::deleteWificredentials();

        // New Feature added for Turn ON BLE Mode after pressing Reset Button for 5 seconds
        BLE_mode = true;
        WiFi.removeEvent(wifi_gotIP_event);
        WiFi.removeEvent(wifi_reconnect_event);
        WiFi.disconnect();
        WiFi.mode(WIFI_OFF);
        delay(500);

        // switch to Bluetooth mode
        if(!BLE::isStarted()){
          Serial.println("BLE was complete OFF");
          BLE::start();
        }
        Serial.println("Switching to Bluetooth mode");
        rgb.currentMode = MODE::BLE;
        Serial.println("BLE initialized");
        Serial.println("Waiting for new Wifi Credentials...");
        while (WiFi.status() != WL_CONNECTED)
        {
          // Serial.print(".");
          delay(100);
        }
        BLE::stop();
        BLE_mode = false;
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
  HttpStatusCode status_adhan_start;
  if (waitingForRelease && digitalRead(SMBTN_GPIO) == HIGH)
  {
    waitingForRelease = false; // Button released, allow next press
  }

  check_reset_button();

  // if (digitalRead(RESET_GPIO) == LOW) // Check reset button
  // {
  //   if (resetButtonPressStartTime == 0)
  //   {
  //     resetButtonPressStartTime = xTaskGetTickCount();
  //   }
  //   else if ((xTaskGetTickCount() - resetButtonPressStartTime) >= pdMS_TO_TICKS(5000))
  //   {
  //     Serial.println("Reset button pressed for 5 seconds. Restarting...");
  //     delay(1000); // Small delay to avoid accidental reset
  //     // esp_restart(); // Restart the ESP32
  //     Device::deleteMasjidId();
  //     Device::deleteWificredentials();
  //     ESP.restart();
  //   }
  // }
  // else
  // {
  //   resetButtonPressStartTime = 0;
  // }

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
        stop_requested_time = 0;
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
      // Serial.println("\nButton Counter: \n");
      // Serial.println(state);
      switch (state)
      {
        
      case ADHAN_START_REQUESTED:
        // smHttp.startAdhan();
        status_adhan_start = smHttp.startAdhan();
          if (status_adhan_start == Http::Response::OK || status_adhan_start == Http::Response::CREATED) 
        {
          Serial.println("ADHAN STARTED\n");
          state = ADHAN_STARTED; // Button::counter = 2;
          Button::counter = 2; // above line is also same
          rgb_last_mode = rgb.currentMode;
          rgb.currentMode = MODE::ADHAN_STREAMING;
          // Serial.print("\nADHAN_STARTED");
          Button::adhanStartTime = millis();
        } 
        else 
        {
          Serial.print("ADHAN NOT STARTED\n");
          state = UNPRESSED;
          Button::counter = 0;
          if(rgb.currentMode != MODE::ADHAN_NOT_STARTED){
            rgb_last_mode = rgb.currentMode;
          }
          rgb.currentMode = MODE::ADHAN_NOT_STARTED;
        }
        // Button::counter = 1;
        ADHAN_FLAG = false;
        break;
      case ADHAN_STARTED:
        // smHttp.stopAdhan();
        // Button::counter = 0;
        // state = UNPRESSED;
        // // state = ADHAN_STOPPED;
        // Serial.print("\nADHAN_STOP\n");
        // rgb.currentMode = MODE::ADHAN_CALLED;
        // ADHAN_FLAG = false;
        // ignore_release = true;
        // Button::counter = 5;

        if(stop_requested_time == 0){
          // expiration_time = 1800; // For testing purposes only
          smHttp.stopAdhan(&expiration_time);
          stop_requested_time = millis();
          rgb.currentMode = MODE::ADHAN_CALLED;
          Serial.print("ADHAN_STOP_REQUESTED\n");
        }
        if(millis()- stop_requested_time >= expiration_time * 1000){
          state = ADHAN_STOPPED;
          Serial.print("ADHAN_STOPPED\n");
          rgb.currentMode = rgb_last_mode;
          Button::counter = 0;
          stop_requested_time = 0;
          ADHAN_FLAG = false;
        }
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
      state = ADHAN_STOP_REQUESTED;
      Button::counter = 3; // ADHAN_STOP_REQUESTED
      Serial.print("\nADHAN_STOP_REQUESTED");
      Button::adhanStartTime = 0;
      rgb.currentMode = MODE::ADHAN_CALLED;
    }
  }
  else if (Button::counter == ADHAN_STOP_REQUESTED)
      {
      if(stop_requested_time == 0){
        // expiration_time = 1800; // For testing purposes only
        smHttp.stopAdhan(&expiration_time);
        stop_requested_time = millis();
        rgb.currentMode = MODE::ADHAN_CALLED;
        Serial.print("ADHAN_STOP_REQUESTED\n");
      }
      if(millis()- stop_requested_time >= expiration_time * 1000){
        state = ADHAN_STOPPED;
        Serial.print("ADHAN_STOP\n");
        rgb.currentMode = rgb_last_mode;
        Button::counter = 0;
        stop_requested_time = 0;
      }
    }
  // else if (Button::counter == ADHAN_STOP_REQUESTED)
  // {
  //   smHttp.stopAdhan();
  //   Button::counter = 5;
  //   // state = ADHAN_STOPPED;
  //   Serial.print("\n--------ADHAN_STOP\n");
  //   rgb.currentMode = MODE::ADHAN_CALLED;
  // }

  vTaskDelay(pdMS_TO_TICKS(100));
}
         