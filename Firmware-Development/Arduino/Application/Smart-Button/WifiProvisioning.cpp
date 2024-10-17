#include "WifiProvisioning.hpp"
#include "Device.hpp"
#include "BLE.hpp"
#include "SM_Http.hpp"
#include "SM_Nvs.hpp"
#include <WiFi.h>

bool WifiProvisioning::connectWifi(const char *ssid, const char *pass, int retries)
{ 
  if (ssid == NULL || pass == NULL)
  {
    return false;
  }

  Serial.printf("Connecting to %s...\n", ssid);
  WiFi.onEvent(std::bind(&WifiProvisioning::wifiGotIP, this, std::placeholders::_1, std::placeholders::_2), WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.begin(ssid, pass);
  
  while (WiFi.status() != WL_CONNECTED && retries-- > 0)
  {
    delay(1000);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    WiFi.onEvent(std::bind(&WifiProvisioning::wifiReconnect, this, std::placeholders::_1, std::placeholders::_2), WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    return true;
  }
  else
  {
    WiFi.disconnect();
    Serial.println("Failed to connect to WiFi");
  }

  return false;
}

bool WifiProvisioning::connectWifiFromNVS(void)
{
  char ssid[32];
  char pass[32];
  
  bool ret = Sm_Nvs::get(DEVICE_WIFI_CREDENTIALS_NAMESPACE,
                        std::make_pair("ssid", ssid),
                        std::make_pair("pass", pass));   
                                              
  if (!ret)
  {
    Serial.println("No values saved for ssid or password");
    return false;
  }
  
  return connectWifi(ssid, pass);
}

void WifiProvisioning::wifiGotIP(WiFiEvent_t event, WiFiEventInfo_t info) 
{
  // Print network details
  Serial.print("Connected to Wi-Fi network with IP address: ");
  Serial.println(WiFi.localIP());

  SmHttp smHttp;
  int httpscode = smHttp.sendDeviceTelemetry(Device::macAddress, DEVICE_TELEMETRY_RETRY_INTERVAL, DEVICE_TELEMETRY_MAX_RETRIES );

  switch(httpscode)
  {
    case Http::Response::OK:
    case Http::Response::CREATED:
      deviceTelemetryStatus = TELEMETRY_SENT;
      break;

    case Http::Response::DEVICE_NOT_FOUND:
      Serial.println("Device Reset");
      Device::deleteWificredentials();
      ESP.restart();
      break;

    default:
      BLE::initialize();
      break;
  }
}

bool WifiProvisioning::isTelemetrySent(void)
{
  return (deviceTelemetryStatus == TELEMETRY_SENT);
}

void WifiProvisioning::wifiReconnect(WiFiEvent_t event, WiFiEventInfo_t info)
{
  Serial.println("Disconnected from WiFi ");
  Serial.print("WiFi lost connection. Reason: ");
  Serial.println(info.wifi_sta_disconnected.reason);
  Serial.println("Trying to Reconnect");
  WiFi.reconnect();
}