
// # BT_ENABLE
// add new file hug_app in board for partition
// real gpio of this button is 26 while define 16
#include "Sources/BLE.hpp"
#include "Sources/Button.hpp"
#include "Sources/WifiProvisioning.hpp"
#include "Sources/Mqtt.hpp"
#include "Sources/Device.hpp"
#include "Sources/SM_Nvs.hpp"

SET_LOOP_TASK_STACK_SIZE( 16*1024 ); // 16KB

WifiProvisioning wifiProv;
Button button(SMBTN_GPIO);
MQTTClient mqtt(&wifiProv);

void setup() 
{
  Serial.begin(115200);
  Device::checkMacAddress();
  Serial.printf("Arduino Stack was set to %d bytes\n", getArduinoLoopTaskStackSize());
  BLE::setup(&wifiProv);

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

  mqtt.createTask(); 
  button.setup();
  Device::timerSetup();
}

void loop() 
{ 
  button.loop();
  Device::loop();
  Device::memoryMonitoring();
}
