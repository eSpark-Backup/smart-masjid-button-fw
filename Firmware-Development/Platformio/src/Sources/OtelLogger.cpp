// OtelLogger.cpp
#include "SM_Http.hpp"
#include "OtelLogger.hpp"
#include "SM_Nvs.hpp"
#include "SM_Json.hpp"
#include "CertificateHandler.hpp"
#include "Mqtt.hpp"

bool OtelLogger::enabled = true;

bool OtelLogger::deviceTelemetryStatus = false;

String otelObj;
String otelhost;
String otelUsername;
String otelPassword;

void OtelLogger::enable(bool state) {
    enabled = state;
    // Persist in NVS
    // Sm_Nvs::setInt("debug_enabled", state ? 1 : 0);
}

void loadOTELConfigfromfile() {
  const char* json = certificateHandler.get(FILE_JSON);
  Serial.println(json);

    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, json);

    if (error) {
        Serial.print("[ERROR] Failed to parse JSON: ");
        Serial.println(error.c_str());
        return;
    }

    // Check if "otel" object exists
    if (!doc.containsKey("otel")) {
        Serial.println("[ERROR] 'otel' key not found in JSON");
        return;
    }

    JsonObject otel = doc["otel"].as<JsonObject>();

    // Extract fields safely
    if (otel.containsKey("host_name"))
        otelhost = otel["host_name"].as<const char*>();
    else
        Serial.println("[ERROR] 'host_name' missing in 'otel'");

    if (otel.containsKey("username"))
        otelUsername = otel["username"].as<const char*>();
    else
        Serial.println("[ERROR] 'username' missing in 'otel'");

    if (otel.containsKey("password"))
        otelPassword = otel["password"].as<const char*>();
    else
        Serial.println("[ERROR] 'password' missing in 'otel'");

    Serial.println("[OTEL] Config Loaded:");
    Serial.print("host_name: "); Serial.println(otelhost);
    Serial.print("username: "); Serial.println(otelUsername);
    Serial.print("password: "); Serial.println(otelPassword);
}

HttpStatusCode OtelLogger::log(const char* severity, const char* macAddress,const char* ApiKey) {
    if (!enabled && !ping) return 0;
    
    ping = false;

    char payload[512];
    
    std::string mac = macAddress;
    std::string sev = severity;


    char telemetryJson[128];
    snprintf(telemetryJson, sizeof(telemetryJson),
        R"({"battery":"%d","macAddress":"%s"})",
        BATTERY_HEALTH, mac.c_str());

    snprintf(payload, sizeof(payload),
        R"({"resourceLogs":[{"resource":{"attributes":[{"key":"macAddress","value":{"stringValue":"%s"}}]},"scopeLogs":[{"logRecords":[{"severityText":"%s","stringValue":%s}]}]}]})",
        mac.c_str(), sev.c_str(), telemetryJson);

    Serial.printf("[OTEL] Payload: %s\n", payload);
    Serial.printf("[OTEL] Size of Payload: %d\n", sizeof(payload));
    Serial.printf("[OTEL] Size of Payload: %d\n", strlen(payload));

    return sendToOtel(payload, ApiKey);
}

HttpStatusCode OtelLogger::sendToOtel(const char* payload, const char* ApiKey) {
    HTTPClient http;
    WiFiClient client;
    // client.setInsecure(); // skip cert validation
    Serial.println("-----------------------------------[OTEL] Loading OTEL Config from file");
    loadOTELConfigfromfile();
    Serial.println("-----------------------------------[OTEL] Loaded OTEL Config from file");
    String url = otelhost;
    if(!http.begin(client, url)){
        Serial.println("-------------------------------[OTEL] Failed to begin HTTP connection");
        return 0;  
    };
    // http.setAuthorization(otelUsername, otelPassword);
    http.setAuthorization(otelUsername.c_str(), otelPassword.c_str());
    http.addHeader("Content-Type", "application/json");
    // http.addHeader("x-api-key", ApiKey);
    Serial.println("-----------------------------------[OTEL] Sending OTEL log with ApiKey:");
    Serial.println(ApiKey);
    // size_t len = strlen(payload);  // safe for large payloads
    // HttpStatusCode httpCode = http.POST((uint8_t*)payload, len);
    // HttpStatusCode httpCode = http.POST((uint8_t*)payload, strlen(payload));
    HttpStatusCode httpCode = http.POST(String(payload));
    
    if (httpCode == Http::Response::OK) {
        OtelLogger::deviceTelemetryStatus = true;
        Serial.printf("-----------------------------------[OTEL] Response: %d\n", httpCode);
    } else {
        OtelLogger::deviceTelemetryStatus = false;
        Serial.printf("-----------------------------------[OTEL] Error: %d\n", httpCode);
    }
    http.end();
    
    return httpCode;
}