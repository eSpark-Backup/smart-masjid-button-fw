#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "Device.hpp"
#include "esp_bt_main.h"
#include "esp_bt_device.h"

// Retry parameters
#define DEVICE_TELEMETRY_RETRY_INTERVAL 10 // 10 seconds
#define DEVICE_TELEMETRY_MAX_RETRIES   6 // 6 retries
#define WIFI_RETRY  10 // number of times to retry WiFi connection
#define DEVCICE_FOUND 200
#define TO_MS(seconds) ((seconds) * 1000)

typedef enum
{
  TELEMETRY_NOTSENT,
  TELEMETRY_SENT,
} telemetryStatus;

class IWifiProvisioning 
{
  public:
    virtual bool connectWifi(const char* ssid, const char* pass, int retries = WIFI_RETRY) = 0;
    virtual bool isTelemetrySent(void) = 0;
};

class WifiProvisioning : public IWifiProvisioning
{
  private:
    telemetryStatus deviceTelemetryStatus = TELEMETRY_NOTSENT;
    
  public:
    bool connectWifiFromNVS(void);
    bool connectWifi(const char* ssid, const char* pass, int retries = WIFI_RETRY) override;
    void wifiGotIP(WiFiEvent_t event, WiFiEventInfo_t info);
    bool isTelemetrySent(void) override; 
    void wifiReconnect(WiFiEvent_t event, WiFiEventInfo_t info);

    //  telemetryStatus getDeviceTelemetryStatus() const {
    //     return deviceTelemetryStatus;
    // }

    // void setDeviceTelemetryStatus(telemetryStatus status) {
    //     deviceTelemetryStatus = status;
    // }
};
