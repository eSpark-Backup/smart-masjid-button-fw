#include <Arduino.h>
#include "FunctionalInterrupt.h"
#include "Button.hpp"
#include "BLE.hpp"
#include "SM_Http.hpp"
#include <WiFi.h>
#include "Sources/WifiProvisioning.hpp"
#include "esp_wifi.h"
#include "LED.hpp"
#include "OtelLogger.hpp"

WifiProvisioning wifiProvis;
volatile int Button::counter = 0;
bool Button::resetFlag = false;
extern volatile buttonState state = UNPRESSED;
uint32_t Button::adhanStartTime = 0;
TickType_t buttonPressStartTime = 0;
TickType_t resetButtonPressStartTime = 0;
bool resetButtonPressed = 0; // for button pressed once logic
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

void BLE_reinitialize(){
  values_found = false;
        
  BLE_mode = true;
  WiFi.removeEvent(wifi_gotIP_event);
  WiFi.removeEvent(wifi_reconnect_event);
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  delay(500);
  
  if(!BLE::isStarted()){
    Serial.println("BLE was complete OFF");
    BLE::initialize();   // Creates server, services, characteristics
    // BLE::start();
  }
  Serial.println("Switching to Bluetooth mode");
  rgb->currentMode = MODE::BLE;
  Serial.println("BLE initialized");
  Serial.println("Waiting for new Wifi Credentials...");
  while (WiFi.status() != WL_CONNECTED)
  {
    // Serial.print(".");
    delay(100);
  }
  BLE_mode = false;
  BLE::stop();
}

void check_reset_button(){
  if (digitalRead(RESET_GPIO) == LOW) // Check reset button
    {
      if (resetButtonPressStartTime == 0)
      {
        resetButtonPressStartTime = millis();
      } else if ((millis() - resetButtonPressStartTime) >= 5000) // if hold for 5 seconds
      {
        Serial.println("Reset button pressed for 5 seconds. Switching to BLE Mode...");
        OtelLogger::log("INFO", Device::macAddress, "Reset button pressed for 5 seconds. Switching to BLE Mode...");
        delay(1000); // Small delay to avoid accidental reset
        Device::deleteMasjidId();
        Device::deleteWificredentials();

        BLE_reinitialize();

          // ESP.restart();
      }
      
      if(millis() - resetButtonPressStartTime > 100){ // debounce
        resetButtonPressed = true;
      }
      Serial.println("Reset button pressed");
    }
    else
    {
      if (resetButtonPressed && ((millis() - resetButtonPressStartTime) < 5000)) // if pressed once
      {
        Serial.println("Reset button pressed Once. Only BLE Turning ON");
        OtelLogger::log("INFO", Device::macAddress, "Reset button pressed. Only BLE Turning ON");
        
        BLE_reinitialize();

      }
      resetButtonPressed = false;
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
        if(smHttp.resetAdhan() == Http::Response::OK)
        {
          Serial.println("\nAdhan is Reset\n");
          rgb->currentMode = MODE::ADHAN_RESET_MODE;
          Button::resetFlag = true;
          Button::counter = 0;
          buttonPressStartTime = 0;
          button_check_time = 0;
          ADHAN_FLAG = false;
          ignore_release = true;
          stop_requested_time = 0;
          OtelLogger::log("INFO", Device::macAddress, "Adhan is Reset through Button");
        }
        else{
          Serial.println("Adhan Reset Failed");
          OtelLogger::log("ERROR", Device::macAddress, "Adhan Reset Failed");
        }
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
        status_adhan_start = smHttp.startAdhan();
        if (status_adhan_start == Http::Response::OK || status_adhan_start == Http::Response::CREATED) 
        {
        Serial.println("ADHAN STARTED\n");
        state = ADHAN_STARTED; // Button::counter = 2;
        Button::counter = 2; // above line is also same
        rgb_last_mode = rgb->currentMode;
        rgb->currentMode = MODE::ADHAN_STREAMING;
        // Serial.print("\nADHAN_STARTED");
        Button::adhanStartTime = millis();
        stop_requested_time = 0;
        OtelLogger::log("INFO", Device::macAddress, "Adhan Started");
        } 
        else 
        {
          Serial.print("ADHAN NOT STARTED\n");
          state = UNPRESSED;
          Button::counter = 0;
          if(rgb->currentMode != MODE::ADHAN_NOT_STARTED)
          {
            rgb_last_mode = rgb->currentMode;
          }
          rgb->currentMode = MODE::ADHAN_NOT_STARTED;
          OtelLogger::log("ERROR", Device::macAddress, "Adhan Not Started");
        }
        ADHAN_FLAG = false;
        break;

      case ADHAN_STARTED:
        if(stop_requested_time == 0)
        {
          // expiration_time = 1800; // For testing purposes only
          smHttp.stopAdhan(&expiration_time);
          stop_requested_time = millis();
          rgb->currentMode = MODE::ADHAN_CALLED;
          Serial.print("ADHAN_STOP_REQUESTED By Button\n");
          Button::adhanStartTime = 0;
          OtelLogger::log("INFO", Device::macAddress, "Adhan Stop Requested By Button");
        }
        if(millis()- stop_requested_time >= expiration_time * 1000){ // expiration time in seconds * 1000 = milliseconds
          state = ADHAN_STOPPED;
          Serial.print("ADHAN_STOPPED\n");
          OtelLogger::log("INFO", Device::macAddress, "Adhan Stopped");
          rgb->currentMode = rgb_last_mode;
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

  ///////////////////////////////////////////////////////////
  // Auto Stop Adhan after max Adhan duration
  if (Button::counter == ADHAN_STARTED)
  {
    if ((millis() - Button::adhanStartTime) > ADHAN_MAX_DURATION) // if Adhan started then chech Adhan Duration
    {
      // state = ADHAN_STOP_REQUESTED;
      // Button::counter = 3; // ADHAN_STOP_REQUESTED
      
      if(stop_requested_time == 0)
      {
        smHttp.stopAdhan(&expiration_time);
        Serial.print("\nADHAN_STOP_REQUESTED due to Adhan Max Duration"); 
        OtelLogger::log("INFO", Device::macAddress, "Adhan Stop Requested due to Adhan Max Duration");  
        // Button::adhanStartTime = 0;
        rgb->currentMode = MODE::ADHAN_CALLED;
        stop_requested_time = millis();
      }
      if(millis()- stop_requested_time >= expiration_time * 1000)
      {
        state = ADHAN_STOPPED;
        Serial.print("ADHAN_STOPPED Due to Adhan Max Duration\n");
        OtelLogger::log("INFO", Device::macAddress, "Adhan Stopped");
        rgb->currentMode = rgb_last_mode;
        Button::counter = 0;
        stop_requested_time = 0;
        Button::adhanStartTime = 0;
      }
    }
  }
  ///////////////////////////////////////////////////////////


  // else if (Button::counter == ADHAN_STOP_REQUESTED)
  //     {
  //     if(stop_requested_time == 0){
  //       // expiration_time = 1800; // For testing purposes only
  //       smHttp.stopAdhan(&expiration_time);
  //       stop_requested_time = millis();
  //       rgb->currentMode = MODE::ADHAN_CALLED;
  //       Serial.print("ADHAN_STOP_REQUESTED\n");
  //     }
  //     if(millis()- stop_requested_time >= expiration_time * 1000)
  //     {
  //       state = ADHAN_STOPPED;
  //       Serial.print("ADHAN_STOP\n");
  //       OtelLogger::log("INFO", Device::macAddress, "Adhan Stopped");
  //       rgb->currentMode = rgb_last_mode;
  //       Button::counter = 0;
  //       stop_requested_time = 0;
  //     }
  //   }

  vTaskDelay(pdMS_TO_TICKS(100));
}
         