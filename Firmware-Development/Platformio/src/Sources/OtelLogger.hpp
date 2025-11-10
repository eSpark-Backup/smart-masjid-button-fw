// OtelLogger.hpp
#pragma once
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "SM_Http.hpp"

class OtelLogger {
public:
    static bool enabled;
    static bool deviceTelemetryStatus;

    static void enable(bool state);
    static HttpStatusCode log(const char* severity, const char* macAddress,const char* externalApiKey);


private:
    static HttpStatusCode sendToOtel(const char* payload, const char* ApiKey);
};