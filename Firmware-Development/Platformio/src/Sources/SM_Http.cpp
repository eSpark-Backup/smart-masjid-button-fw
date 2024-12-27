#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "SM_Http.hpp"
#include "SM_Json.hpp"
#include "Device.hpp"
#include "BLE.hpp"
#include "Mqtt.hpp"

HttpStatusCode SmHttp::startAdhan() 
{
  std::string startAdhanUrl = smHost;
  startAdhanUrl += adhanStartEndpoint;

  std::string jsonString = SM_Json::create("masjidId", &MASJID_TEST_ID);

  return request(startAdhanUrl.c_str(), jsonString.c_str());
}

HttpStatusCode SmHttp::sendDeviceTelemetry(const char* macAddress, uint8_t retryInterval, uint16_t maxRetry) 
{
  HttpStatusCode status = Http::Response::UNKNOWN;;
  while (maxRetry--) 
  {
    status = sendDeviceTelemetry(macAddress);
    if (status == Http::Response::OK || status == Http::Response::CREATED)
    {
      break;
    }
    else 
    {
      Serial.println("Failed to send telemetry, retrying in 10 seconds...");
      delay(TO_MS(retryInterval));
    }
  }
  return status;
}

HttpStatusCode SmHttp::sendDeviceTelemetry(const char* macAddress) 
{
  std::string deviceTelemetryUrl = smHost;
  deviceTelemetryUrl += telemetryEndpoint;

  char telemetryJson[64];
  SM_Json::create(telemetryJson, sizeof(telemetryJson),
                  std::make_pair("battery", BATTERY_HEALTH),
                  std::make_pair("macAddress", Device::macAddress));
  Serial.println(telemetryJson);

  return request(deviceTelemetryUrl.c_str(), telemetryJson);
}

HttpStatusCode SmHttp::stopAdhan() 
{
  std::string stopAdhanUrl = smHost;
  stopAdhanUrl += adhanStopEndpoint;

  std::string jsonString = SM_Json::create("masjidId", &MASJID_TEST_ID);
  Serial.println(jsonString.c_str());

  return request(stopAdhanUrl.c_str(), jsonString.c_str());
}

HttpStatusCode SmHttp::resetAdhan() 
{
  if (WiFi.status() != WL_CONNECTED)
    return false;

  std::string resetAdhanUrl = smHost;
  resetAdhanUrl += adhanResetEndpoint;

  std::string jsonString = SM_Json::create("masjidId", &MASJID_TEST_ID);
  Serial.println(jsonString.c_str());

  return request(resetAdhanUrl.c_str(), jsonString.c_str(), Http::RequestType::PATCH);
}

HttpStatusCode SmHttp::notifyOTAUpdate(const char* macAddress) 
{  
  std::string updateOnLoadUrl = smHost;
  updateOnLoadUrl += OtaUpdateEndpoint;

  char updateOnLoadUrlJson[64];
  snprintf(updateOnLoadUrlJson, sizeof(updateOnLoadUrlJson), "{\"macAddress\":\"%s\",\"firmwareVersion\":\"%s\"}", Device::macAddress, SM_FIRMWARE_VER);
  // SM_Json::create(updateOnLoadUrlJson, sizeof(updateOnLoadUrlJson), std::make_pair("macAddress", Device::macAddress), std::make_pair("firmwareVersion", SM_FIRMWARE_VER));
  Serial.println(updateOnLoadUrlJson);
  return request(updateOnLoadUrl.c_str(), updateOnLoadUrlJson, Http::RequestType::PUT);
}

HttpStatusCode SmHttp::updateDeviceStatus(const char* macAddress, char* status)
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Wifi not connected : Unable to send request");
    return false;
  }

  std::string updateOnLoadUrl = smHost;
  updateOnLoadUrl += otaEndpoint;

  char payload[64];
  SM_Json::create(payload, sizeof(payload),
                  std::make_pair("macAddress", Device::macAddress),
                  std::make_pair("status", status));
  Serial.println(payload);
  return request(updateOnLoadUrl.c_str(), payload, Http::RequestType::PUT);
}

HttpStatusCode SmHttp::request(const char* url, const char* json, Http::RequestType request) 
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Wifi not connected : Unable to send request");
    return false;
  }

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient https;
  https.setReuse(false);
  https.begin(client, url);
  https.addHeader("Content-Type", "application/json");
  https.addHeader("x-api-key", EXTERNAL_API_KEY);
  HttpStatusCode status = Http::Response::UNKNOWN;;
  Serial.print("\nWIFI RSSI : ");
  Serial.println(WiFi.RSSI());
  switch(request)
  {
    case Http::RequestType::POST:
       status = https.POST(json);
       break;
    case Http::RequestType::PATCH:
       status = https.PATCH(json);
       break;
    case Http::RequestType::PUT: 
        status = https.PUT(json);
        break;  
  }
  
  const char* response = https.getString().c_str();
  https.end();

  if (status == Http::Response::OK || status == Http::Response::CREATED) 
  {
    Serial.println("Request successful!");
  } 
  else 
  {
    Serial.print("Request failed with error code: ");
    Serial.println(status);
  }

  Serial.println(response);
  return status;
}
