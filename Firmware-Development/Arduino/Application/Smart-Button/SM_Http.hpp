#pragma once

#include <WiFiClientSecure.h>
#include "Mqtt.hpp"

#define EXTERNAL_API_KEY "b8z6X5fzbjNDGHz15XtPVzEkO3h5mCT7q4Ei8AupX9DD12PxL94rhnZHarHc8O2H"
#define TO_MS(seconds) (seconds * 1000)

typedef int HttpStatusCode;

namespace Http
{
  enum Response
  {
    OK = 200,
    CREATED = 201,
    DEVICE_NOT_FOUND = 404,
  };
  
  enum class RequestType
  {
    POST,
    PATCH,
    PUT,
  };
}

class SmHttp 
{
  private:
    const char* smHost = "https://smartmasjid-api-qa.futuriklabs.com";
    const char* telemetryEndpoint = "/external/device/telemetry";
    const char* adhanStartEndpoint = "/external/adhan/start";
    const char* adhanStopEndpoint = "/external/adhan/stop";
    const char* adhanResetEndpoint = "/external/adhan/reset";
    const char* OtaUpdateEndpoint = "/external/device/update";
    const char* otaEndpoint = "/external/device/ota";
    char MASJID_TEST_ID = '3';

  public:
    
    HttpStatusCode startAdhan();  
    HttpStatusCode sendDeviceTelemetry(const char* macAddress, uint8_t retryInterval, uint16_t maxRetryDuration);
    HttpStatusCode sendDeviceTelemetry(const char* macAddress);
    HttpStatusCode stopAdhan(); 
    HttpStatusCode resetAdhan();
    HttpStatusCode notifyOTAUpdate(const char* macAddress);
    HttpStatusCode updateDeviceStatus(const char* macAddress, char* status);
    HttpStatusCode request(const char* url, const char* json, Http::RequestType request = Http::RequestType::POST);
};
