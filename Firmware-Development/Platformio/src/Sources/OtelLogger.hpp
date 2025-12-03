// OtelLogger.hpp
#pragma once
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "SM_Http.hpp"


extern String otelObj;
extern String otelhost;
extern String otelendpoint;
extern String otelUsername;
extern String otelPassword;

void loadOTELConfigfromfile();

class OtelLogger {
public:
    static bool enabled;
    static bool deviceTelemetryStatus;

    static void enable(bool state);
    static HttpStatusCode log(const char* severity, const char* macAddress, const char* messsage);


private:
    static HttpStatusCode sendToOtel(const char* payload);
};