// OtelLogger.cpp
#include "SM_Http.hpp"
#include "OtelLogger.hpp"
#include "SM_Nvs.hpp"
#include "SM_Json.hpp"
#include "CertificateHandler.hpp"
#include "Mqtt.hpp"

bool OtelLogger::enabled = false;

bool OtelLogger::deviceTelemetryStatus = false;

String otelObj;
String otelhost;
String otelendpoint;

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

    loadConfigFromFile();

    if (error) {
        Serial.print("[ERROR] Failed to parse JSON: ");
        Serial.println(error.c_str());
        OtelLogger::log("ERROR", Device::macAddress, "Failed to read config.json");
        return;
    }

    // Check if "otel" object exists
    if (!doc.containsKey("otel")) {
        Serial.println("[ERROR] 'otel' key not found in JSON");
        OtelLogger::log("ERROR", Device::macAddress, " 'otel' key not found in JSON");
        
        return;
    }

    JsonObject otel = doc["otel"].as<JsonObject>();

    // Extract fields safely
    if (otel.containsKey("host_name"))
        otelhost = otel["host_name"].as<const char*>();
    else
        Serial.println("[ERROR] 'host_name' missing in 'otel'");

    Serial.println("[OTEL] Config Loaded:");
    Serial.print("host_name: "); Serial.println(otelhost);
    OtelLogger::log("INFO", Device::macAddress, "[OTEL] Config Loaded from config.json");
}

SmHttp sm_http;
HttpStatusCode OtelLogger::log(const char* severity, const char* macAddress, const char* messsage) {
    if (!enabled && !ping) return 0;
    
    ping = false;

    char payload[512];

    // Old Format
    // snprintf(payload, sizeof(payload),
    //     R"({"macAddress":[{"resource":{"attributes":[{"key":"macAddress","value":{"stringValue":"%s"}}]},"scopeLogs":[{"logRecords":[{"severityText":"%s","stringValue":"%s"}]}]}]})",
    //     macAddress, severity, messsage);


// new format        
// { "macAddress": "9C:B8:B4:31:FD:98", "logType": "INFO", "logText": "This is a test log!" }
    snprintf(payload, sizeof(payload),
        R"({"macAddress":"%s","logType":"%s","logText":"%s"})",
        macAddress, severity, messsage);


    Serial.printf("-------------------------------[OTEL] Payload: %s\n", payload);
    // Serial.printf("[OTEL] Size of Payload: %d\n", sizeof(payload)); // Limit
    // Serial.printf("[OTEL] Size of Payload: %d\n", strlen(payload)); // Actual Size

    // Old method
    // return sendToOtel(payload);

    // New Method
    std::string deviceOtelUrl = std::string(otelhost.c_str());
    deviceOtelUrl += sm_http.getOtelEndpoint();
    // deviceOtelUrl += otelendpoint;

    return sm_http.request(deviceOtelUrl.c_str(), payload);
}
HttpStatusCode OtelLogger::sendToOtel(const char* payload) {
    HTTPClient http;
    WiFiClient client;
    // client.setInsecure(); // skip cert validation
    // Serial.println("-----------------------------------[OTEL] Loading OTEL Config from file");
    // loadOTELConfigfromfile();
    // Serial.println("-----------------------------------[OTEL] Loaded OTEL Config from file");
    String url = otelhost;
    if(!http.begin(client, url)){
        Serial.println("-------------------------------[OTEL] Failed to begin HTTP connection");
        return 0;  
    };
    // http.setAuthorization(otelUsername, otelPassword);
    http.setAuthorization(otelUsername.c_str(), otelPassword.c_str());
    http.addHeader("Content-Type", "application/json");
    // HttpStatusCode httpCode = http.POST((uint8_t*)payload, len);
    // HttpStatusCode httpCode = http.POST((uint8_t*)payload, strlen(payload));
    HttpStatusCode httpCode = http.POST(String(payload));
    Serial.printf("-----------------------------------[OTEL] Response: %d\n", httpCode);
    if (httpCode == Http::Response::OK) {
        OtelLogger::deviceTelemetryStatus = true;
    } else {
        OtelLogger::deviceTelemetryStatus = false;
    }
    http.end();
    
    return httpCode;
}