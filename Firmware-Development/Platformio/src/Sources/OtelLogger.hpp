// OtelLogger.hpp
#pragma once
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

class OtelLogger {
public:
    static bool enabled;

    static void enable(bool state);
    static void log(const char* severity, const char* message, const char* macAddress);

private:
    static void sendToOtel(const char* payload);
};
