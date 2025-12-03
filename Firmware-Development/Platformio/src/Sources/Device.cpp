#include <WiFi.h>
#include "SM_Nvs.hpp"
#include "SM_Http.hpp"
#include "Device.hpp"
#include "esp_bt_device.h"
#include "OtelLogger.hpp"

char Device::macAddress[20] = "";
bool Device::telemetryTrigger = false; 
bool Device::otaUpdating = false; 
hw_timer_t* Device::timer = NULL; 

void IRAM_ATTR Device::triggerTelemetryUpdate() 
{
  telemetryTrigger= true; 
}

void Device::setup() 
{
  if(checkMacAddress())
  {
   Serial.println(Device::macAddress);
  }
  else
  {
    Device::getDeviceMacAddress(Device::macAddress);
    storeMacAddress();
  }
}

void Device::timerSetup() 
{
  Device::timer = timerBegin(TIMER_NO, TIMER_PRESCALAR_1MHZ, true);
  timerAttachInterrupt(Device::timer, &Device::triggerTelemetryUpdate, true); 
  timerAlarmWrite(Device::timer, TELEMETRY_UPDATE_TIME * TIMER_ONE_SECOND_COUNTS, true);
  timerAlarmEnable(Device::timer);
}

void Device::loop() 
{
  if(telemetryTrigger && !Device::otaUpdating)
  {
    Serial.println("OTA is not Updating....");
    OtelLogger::log("INFO", Device::macAddress, "Updating Telemetry");
    Serial.println("Updating Telemetry");
    SmHttp smHttp;
    smHttp.sendDeviceTelemetry(Device::macAddress);
    telemetryTrigger= false;
  }
}

bool Device::deleteMasjidId() 
{
  if (!Sm_Nvs::del(DEVICE_FACTORY_INFO_NAMESPACE, "masjidId")) 
    return false;
    
  Serial.println("Masjid Id Removed");
  
  return true;
}

bool Device::deleteWificredentials() 
{
  if (!Sm_Nvs::del(DEVICE_WIFI_CREDENTIALS_NAMESPACE, "ssid", "pass")) 
    return false;
    
  Serial.println("Old credentials removed");
  OtelLogger::log("INFO", Device::macAddress, "Old credentials removed");
  return true;
}

bool Device::updateWifiCredential(const char* ssid, const char* pass) 
{
  if (!Sm_Nvs::put(DEVICE_WIFI_CREDENTIALS_NAMESPACE,
                  std::make_pair("ssid", ssid),
                  std::make_pair("pass", pass))) 
    return false;
    
  Serial.println("Wifi credentials updated");
  OtelLogger::log("INFO", Device::macAddress, "Wifi credentials updated in NVS");
  return true;
}

bool Device::checkMacAddress()
{
  if (Sm_Nvs::get(DEVICE_FACTORY_INFO_NAMESPACE, std::make_pair("macaddress", Device::macAddress)))
  {
    return true;
  }
  return false;
}

bool Device::storeMacAddress()
{
  
  if (Sm_Nvs::put(DEVICE_FACTORY_INFO_NAMESPACE, std::make_pair("macaddress", Device::macAddress)))
  {
    Serial.print("Updated MAC address in NVS: ");
    Serial.println(Device::macAddress);
    return true;
  }
  else
  {
    Serial.println("Failed to update MAC address in NVS.");
    return false;
  }
}

void Device::getDeviceMacAddress(char* bleMacAddress) 
{
  const uint8_t* point = esp_bt_dev_get_address();

  if (point != NULL)
    sprintf(bleMacAddress, "%02X:%02X:%02X:%02X:%02X:%02X", (int)point[0],(int)point[1],(int)point[2],(int)point[3],(int)point[4],(int)point[5]);
}

void Device::memoryMonitoring() 
{
  unsigned long currentTime = millis();
  static unsigned long lastExecution = 0;
  if (currentTime - lastExecution >= DEVICE_MEMORY_INFO_PRINT_INTERVAL) // after 5 minutes
   {
    Serial.print("1- Free Stack Space: ");
    Serial.println(uxTaskGetStackHighWaterMark(NULL));
    Serial.print("2- Free Heap Space: ");
    Serial.println(ESP.getFreeHeap());
    Serial.print("3- Maximum allocated Heap : ");
    Serial.println(ESP.getMaxAllocHeap());
    Serial.print("4- Free Heap Space: ");
    Serial.println(ESP.getFreeHeap());
    Serial.print("5- Minimum Free Heap Space: ");
    Serial.println(esp_get_minimum_free_heap_size());
    Serial.print("6- Total PSRAM: ");
    Serial.println(ESP.getPsramSize() / 1024);
    Serial.print(" KB");
    Serial.print("7- Free PSRAM: ");
    Serial.println(ESP.getFreePsram() / 1024);
    Serial.println(" KB ");
    
    char buf[512];
    
    snprintf(buf, sizeof(buf),"Memory Monitor 1- Free Stack Space: %u 2- Free Heap Space: %u 3- Maximum allocated Heap : %u 4- Free Heap Space: %u 5- Minimum Free Heap Space: %u 6- Total PSRAM: %u 7- Free PSRAM: %u" ,uxTaskGetStackHighWaterMark(NULL), ESP.getFreeHeap(), ESP.getMaxAllocHeap(), ESP.getFreeHeap(), esp_get_minimum_free_heap_size(), ESP.getPsramSize() / 1024, ESP.getFreePsram() / 1024);

    OtelLogger::log("INFO", Device::macAddress, buf);

    lastExecution = currentTime;
    
  }
}
