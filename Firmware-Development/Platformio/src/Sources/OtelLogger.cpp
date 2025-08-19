// OtelLogger.cpp
#include "OtelLogger.hpp"
#include "SM_Nvs.hpp"
#include "SM_Json.hpp"
#include "CertificateHandler.hpp"

bool OtelLogger::enabled = false;

char otelObj[120];
char otelhost[40];
char otelUsername[8];
char otelPassword[8];

void OtelLogger::enable(bool state) {
    enabled = state;
    // Persist in NVS
    // Sm_Nvs::setInt("debug_enabled", state ? 1 : 0);
}

void loadOTELConfigfromfile() {
  const char* json = certificateHandler.get(FILE_JSON);
  SM_Json::parse(json , std::make_pair("otel", otelObj));
  SM_Json::parse((const char*)(otelObj) , std::make_pair("host_name", otelhost));
  SM_Json::parse((const char*)(otelObj) , std::make_pair("username", otelUsername));
  SM_Json::parse((const char*)(otelObj) , std::make_pair("password", otelPassword));
}

void OtelLogger::log(const char* severity, const char* message, const char* macAddress) {
    if (!enabled) return;

    char payload[512];
    snprintf(payload, sizeof(payload),
        R"({
            "resourceLogs": [{
                "resource": {
                    "attributes": [{
                        "key": "macAddress",
                        "value": { "stringValue": "%s" }
                    }]
                },
                "scopeLogs": [{
                    "logRecords": [{
                        "severityText": "%s",
                        "body": { "stringValue": "%s" }
                    }]
                }]
            }]
        })",
        macAddress, severity, message
    );

    sendToOtel(payload);
}

void OtelLogger::sendToOtel(const char* payload) {
    HTTPClient http;
    WiFiClientSecure client;
    client.setInsecure(); // skip cert validation
    loadOTELConfigfromfile();
    String url = otelhost;
    http.begin(client, url);
    http.setAuthorization(otelUsername, otelPassword);
    http.addHeader("Content-Type", "application/json");

    int httpCode = http.POST((uint8_t*)payload, strlen(payload));
    if (httpCode > 0) {
        Serial.printf("[OTEL] Response: %d\n", httpCode);
    } else {
        Serial.printf("[OTEL] Error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
}
