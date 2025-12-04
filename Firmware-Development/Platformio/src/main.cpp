
// # BT_ENABLE
// add new file hug_app in board for partition
// real gpio of this button is 26 while define 16 //// removed this
#include "esp_system.h"
#include "esp_sleep.h"
#include "Sources/BLE.hpp"
#include "Sources/Button.hpp"
#include "Sources/WifiProvisioning.hpp"
#include "Sources/Mqtt.hpp"
#include "Sources/Device.hpp"
#include "Sources/SM_Nvs.hpp"
#include "Sources/CertificateHandler.hpp"
#include "Sources/WifiMonitoring.hpp"
#include "Sources/OTELLogger.hpp"
#include "Sources/LED.hpp"


SET_LOOP_TASK_STACK_SIZE( 16*1024 ); // 16KB

WifiProvisioning wifiProv;
Button button(SMBTN_GPIO);
MQTTClient mqtt(&wifiProv);
WifiMonitoring wifiMonitor;
// CertificateHandler certificateHandler;

void setup() 
{
  Serial.begin(115200);
  rgb = LED::getInstance();
  rgb->createTask();
  rgb->currentMode = MODE::WIFI_DISCONNECTED; 
  certificateHandler.setup();
  Device::checkMacAddress();
  Serial.printf("Arduino Stack was set to %d bytes\n", getArduinoLoopTaskStackSize());
  button.setup();
  BLE::setup(&wifiProv);
  
  // Serial.println("------------- TESTING---------------");
  // Serial.println("Going to sleep for 10 seconds");
  // delay(1000);
  // esp_sleep_enable_timer_wakeup(10 * 1000000); // 10 seconds in microseconds
  // esp_light_sleep_start();
  // Serial.println("Woke up from sleep after 10 seconds");
  // Serial.println("------------- TESTING END---------------");

  if(wifiProv.connectWifiFromNVS())
  {

    while(!wifiProv.isTelemetrySent())
    {
      delay(100);
    }
  } 
  else
  {
    // Turn off wifi
    WiFi.mode(WIFI_OFF);
    delay(100);
    
    // switch to Bluetooth mode
    Serial.println("Switching to Bluetooth mode");
    BLE::initialize();
    BLE_mode = true;
    while(WiFi.status() != WL_CONNECTED) 
    {
      delay(100);
    }
    BLE::stop();
    BLE_mode = false;
  } 

  while(!wifiProv.isTelemetrySent())
  {
    delay(100);
  }

  
  certificateHandler.print();

  mqtt.createTask(); 
  Device::timerSetup();

  loadOTELConfigfromfile();
  OtelLogger::log("INFO", Device::macAddress, "Device Setup Complete");
}

void loop() 
{ 
  wifiMonitor.MonitorWifi();
  button.loop();
  Device::loop(); 
  Device::memoryMonitoring();
}
