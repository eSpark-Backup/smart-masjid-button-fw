
// # BT_ENABLE
// add new file hug_app in board for partition
// real gpio of this button is 26 while define 16
#include "esp_system.h"
#include "esp_sleep.h"
#include "Sources/BLE.hpp"
#include "Sources/Button.hpp"
#include "Sources/WifiProvisioning.hpp"
#include "Sources/Mqtt.hpp"
#include "Sources/Device.hpp"
#include "Sources/SM_Nvs.hpp"
#include  "Sources/CertificateHandler.hpp"
#include "Sources/WifiMonitoring.hpp"


SET_LOOP_TASK_STACK_SIZE( 16*1024 ); // 16KB

WifiProvisioning wifiProv;
Button button(SMBTN_GPIO);
MQTTClient mqtt(&wifiProv);
WifiMonitoring wifiMonitor;

// CertificateHandler certificateHandler;


void setup() 
{
  Serial.begin(115200);
  LED& rgb = *LED::getInstance();
  rgb.createTask();
  rgb.currentMode = MODE::ON_POWER_UP; 
  certificateHandler.setup();
  Serial.printf("Arduino Stack was set to %d bytes\n", getArduinoLoopTaskStackSize());
  Device::checkMacAddress();
  BLE::setup(&wifiProv);

  // deep sleep is in connectWifi() function

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
    while(WiFi.status() != WL_CONNECTED) 
    {
      delay(100);
    }
    BLE::stop();
  }
  
  while(!wifiProv.isTelemetrySent())
  {
    delay(100);
  }

  
  certificateHandler.print();

  mqtt.createTask(); 
  button.setup();
  Device::timerSetup();
}

void loop() 
{ 
  wifiMonitor.MonitorWifi();
  button.loop();
  Device::loop();
  Device::memoryMonitoring();
}
