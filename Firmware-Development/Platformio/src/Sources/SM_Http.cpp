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
#include "SM_Nvs.hpp"
#include "OtelLogger.hpp"
char http[150];
char smHost[50];
char externalApiKey[70];
char device[70];
char smFirmware[6];

Json_Urls json_urls;

void loadConfigFromFile() {
  const char* json = certificateHandler.get(FILE_JSON);
  SM_Json::parse(json, std::make_pair("http", http));
  SM_Json::parse((const char*)(http), std::make_pair("host", smHost));
  SM_Json::parse((const char*)(http) , std::make_pair("api_key", externalApiKey));
  SM_Json::parse(json , std::make_pair("device_info", device));
  SM_Json::parse((const char*)(device) , std::make_pair("firmware_ver", smFirmware));
}

const char* SmHttp::getOtelEndpoint(){
  return otelEndpoint;
}

HttpStatusCode SmHttp::startAdhan() 
{
  loadConfigFromFile();
  std::string startAdhanUrl = smHost;
  startAdhanUrl += adhanStartEndpoint;
  
  char masjidId[32];
  Sm_Nvs::get(DEVICE_FACTORY_INFO_NAMESPACE,
    std::make_pair("masjidId", masjidId));

  std::string jsonString = SM_Json::create("masjidId", masjidId);
  Serial.println("\n\n*-----------------------------------\n\n");
  Serial.println("\nStarting Adhan with payload: ");
  Serial.println(jsonString.c_str());

  return request(startAdhanUrl.c_str(), jsonString.c_str());
}

HttpStatusCode SmHttp::sendDeviceTelemetry(const char* macAddress, uint8_t retryInterval, uint16_t maxRetry) 
{
  HttpStatusCode status = Http::Response::UNKNOWN;
  if(maxRetry == DEVICE_TELEMETRY_MAX_RETRIES){
    delay(TO_MS(2)); // initial delay to wait for device add in the backend
  }
  while (maxRetry--) 
  {
    status = sendDeviceTelemetry(macAddress);
    if (status == Http::Response::OK || status == Http::Response::CREATED)
    {
      break;
    }
    else 
    {
      Serial.print("-----------------Failed to send telemetry ");
      Serial.print(maxRetry);
      Serial.println(" , retrying in 10 seconds...");
      delay(TO_MS(retryInterval));
    }
  }
  return status;
}

HttpStatusCode SmHttp::sendDeviceTelemetry(const char* macAddress) 
{
  
  loadConfigFromFile();

  std::string deviceTelemetryUrl = smHost;
  deviceTelemetryUrl += telemetryEndpoint;

  char telemetryJson[64];
  SM_Json::create(telemetryJson, sizeof(telemetryJson),
                  std::make_pair("battery", BATTERY_HEALTH),
                  std::make_pair("macAddress", Device::macAddress));
  Serial.println(telemetryJson);

  return request(deviceTelemetryUrl.c_str(), telemetryJson);
  // return OtelLogger::log("INFO", Device::macAddress, externalApiKey);
  
}

// HttpStatusCode SmHttp::stopAdhan() 
// {
//   std::string stopAdhanUrl = smHost;
//   stopAdhanUrl += adhanStopEndpoint;
  
//   char masjidId[32];
//   Sm_Nvs::get(DEVICE_FACTORY_INFO_NAMESPACE,
//     std::make_pair("masjidId", masjidId));

//   std::string jsonString = SM_Json::create("masjidId", masjidId);
//   Serial.println(jsonString.c_str());

//   return request(stopAdhanUrl.c_str(), jsonString.c_str());
// }

HttpStatusCode SmHttp::stopAdhan(int32_t *expiration_time) 
{
  std::string stopAdhanUrl = smHost;
  stopAdhanUrl += adhanStopEndpoint;
  
  char masjidId[32];
  Sm_Nvs::get(DEVICE_FACTORY_INFO_NAMESPACE,
  std::make_pair("masjidId", masjidId));

  std::string jsonString = SM_Json::create("masjidId", masjidId);
  Serial.println(jsonString.c_str());
  
  String payload = "{}";

  HttpStatusCode status = request(stopAdhanUrl.c_str(), jsonString.c_str(), Http::RequestType::POST, &payload);
  if (payload.length() > 0) {
    StaticJsonDocument<256> doc;
    DeserializationError err = deserializeJson(doc, payload);

    if (!err && doc.containsKey("adhanCalledExpiration")) {
      *expiration_time = doc["adhanCalledExpiration"];
      Serial.print("adhanCalledExpiration: ");
      Serial.print(*expiration_time);
      Serial.println(" seconds");
    } else {
      Serial.println("Failed to parse JSON or missing key.");
    }
  } else {
    Serial.println("Empty response received.");
  }

  return status;
}


HttpStatusCode SmHttp::resetAdhan() 
{
  loadConfigFromFile();
  if (WiFi.status() != WL_CONNECTED)
    return false;

  std::string resetAdhanUrl = smHost;
  resetAdhanUrl += adhanResetEndpoint;

  char masjidId[32];
  Sm_Nvs::get(DEVICE_FACTORY_INFO_NAMESPACE,
    std::make_pair("masjidId", masjidId));

  std::string jsonString = SM_Json::create("masjidId", masjidId);
  Serial.println(jsonString.c_str());

  return request(resetAdhanUrl.c_str(), jsonString.c_str(), Http::RequestType::PATCH);
}

HttpStatusCode SmHttp::notifyOTAUpdate(const char* macAddress) 
{  
  loadConfigFromFile();
  std::string updateOnLoadUrl = smHost;
  updateOnLoadUrl += OtaUpdateEndpoint;

  char updateOnLoadUrlJson[64];
  snprintf(updateOnLoadUrlJson, sizeof(updateOnLoadUrlJson), "{\"macAddress\":\"%s\",\"firmwareVersion\":\"%s\"}", Device::macAddress, smFirmware);
  // SM_Json::create(updateOnLoadUrlJson, sizeof(updateOnLoadUrlJson), std::make_pair("macAddress", Device::macAddress), std::make_pair("firmwareVersion", SM_FIRMWARE_VER));
  Serial.println("Notifying OTA update with payload: ");
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
  Serial.println("Updating device status with payload: ");
  Serial.println(payload);
  return request(updateOnLoadUrl.c_str(), payload, Http::RequestType::PUT);
}

HttpStatusCode SmHttp::request(const char* url, const char* json, Http::RequestType request) 
{
  Serial.println("Request 1 called");
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
  https.addHeader("x-api-key", externalApiKey);
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



HttpStatusCode SmHttp::request(const char* url, const char* json, Http::RequestType request, String* responseOut) 
{
  Serial.println("Request 2 called");
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
  https.addHeader("x-api-key", externalApiKey);
  HttpStatusCode status = Http::Response::UNKNOWN;;
  Serial.print("\nWIFI RSSI : ");
  Serial.println(WiFi.RSSI());
  switch(request)
  {
    case Http::RequestType::POST:
       status = https.POST(json);
      //  *payload = https.getString();
       break;
    case Http::RequestType::PATCH:
       status = https.PATCH(json);
       break;
    case Http::RequestType::PUT: 
        status = https.PUT(json);
        break;  
  }
  
  if (responseOut != nullptr) {
    *responseOut = https.getString();
    Serial.println(*responseOut);
  }
  https.end();
  // const char* response = https.getString().c_str();
  
  
  if (status == Http::Response::OK || status == Http::Response::CREATED) 
  {
    Serial.println("Request successful!");
  } 
  else 
  {
    Serial.print("Request failed with error code: ");
    Serial.println(status);
  }
  
  return status;
}
