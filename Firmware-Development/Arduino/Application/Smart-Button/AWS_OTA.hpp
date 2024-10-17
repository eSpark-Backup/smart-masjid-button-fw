#pragma once
#include"Arduino.h"
// This sketch provide the functionality of OTA Firmware Upgrade
#include "WiFi.h"
#include "HttpsOTAUpdate.h"

#define OTA_ROLL_BACK "R"
#define OTA_SUCCESS "C"

static const char *server_certificate;
static HttpsOTAStatus_t otastatus;
void awsOtaTask(void* pvParameters);

class AWS_OTA
{ 
  public:

    AWS_OTA() {}
    AWS_OTA(const AWS_OTA&) = delete;
    AWS_OTA& operator=(const AWS_OTA&) = delete;
    static AWS_OTA* getInstance(); 
    static AWS_OTA* instance;
    const char* deviceOtaUrl;
    bool updated = true;
    static void httpEventHandler(HttpEvent_t *event);
    void setup();
    bool getStatus();
    void startFirmwareUpdate(const char* url);
};
