#include"AWS_OTA.hpp"
#include "SM_Http.hpp"
#include"CertificateHandler.hpp"
#include "Device.hpp"
#include "HttpsOTAUpdate.cpp"
#include "SM_Json.hpp"
#include <Update.h>
#include "SPIFFS.h"
#include "LED.hpp"
#include "Button.hpp"

AWS_OTA* AWS_OTA::instance = nullptr;

AWS_OTA* AWS_OTA::getInstance()
{
  if (!instance) 
      {
        instance = new AWS_OTA();
      }
      return instance;
}

void AWS_OTA::httpEventHandler(HttpEvent_t *event) 
{
  switch (event->event_id) 
  {
    case HTTP_EVENT_ERROR:        Serial.println("Http Event Error"); break;
    case HTTP_EVENT_ON_CONNECTED: Serial.println("Http Event On Connected"); break;
    case HTTP_EVENT_HEADER_SENT:  Serial.println("Http Event Header Sent"); break;
    case HTTP_EVENT_ON_HEADER:    Serial.printf("Http Event On Header, key=%s, value=%s\n", event->header_key, event->header_value); break;
    case HTTP_EVENT_ON_DATA:      break;
    case HTTP_EVENT_ON_FINISH:    Serial.println("Http Event On Finish"); break;
    case HTTP_EVENT_DISCONNECTED: Serial.println("Http Event Disconnected"); break;
    // case HTTP_EVENT_REDIRECT:     Serial.println("Http Event Redirect"); break;
  }
}

void AWS_OTA::setup()
{
  config.buffer_size = 2048;
  config.buffer_size_tx = 2048;
  HttpsOTA.onHttpEvent(httpEventHandler);
  Serial.println("Starting OTA");
  HttpsOTA.begin(deviceOtaUrl, server_certificate);
  Serial.println("Please Wait it takes some time ...");
 
}

bool AWS_OTA::getStatus()
{
  otastatus = HttpsOTA.status();
  Serial.print("Ota Status = ");
  Serial.println(otastatus);
  
  switch (otastatus)
  {
    case HTTPS_OTA_SUCCESS:
    Serial.println("Firmware written successfully. To reboot device, call API ESP.restart() or PUSH restart button on device");
    
      return true;

    case HTTPS_OTA_FAIL:
      Serial.println("Firmware Upgrade Fail");
      updated = false;
      if (rgb->currentMode == MODE::ADHAN_STREAMING) {
        rgb->currentMode = MODE::ADHAN_STARTED_INTERNET_STOPPED;
      }
      else{
        rgb->currentMode = rgb_last_mode;
      }
      return false;

    case HTTPS_OTA_UPDATING:
      Serial.println("HTTPS OTA is UPDATING");
      break;

    case HTTPS_OTA_IDLE:
      Serial.println("HTTPS OTA is IDLE");
      break;

    case HTTPS_OTA_ERR:
      Serial.println("HTTPS OTA ERROR");
      rgb->currentMode = rgb_last_mode;
      break;

      
    default:
      Serial.println("Unknown OTA Status");
      rgb->currentMode = rgb_last_mode;
      break;
  }
  return false;
}

void AWS_OTA::startFirmwareUpdate(const char* url)
{
  deviceOtaUrl = url;
  Serial.print("start Firmware Update from URL: ");
  Serial.println(deviceOtaUrl);

   if(rgb->currentMode != rgb_last_mode){
    rgb_last_mode = rgb->currentMode;
  }
  rgb->currentMode = MODE::OTA_IN_PROGRESS;

  // CertificateHandler certificateHandler;
  certificateHandler.setup();
  certificateHandler.isExist();
  //certificateHandler.write(server_certificate);  // Write new content in  to the file
  server_certificate = certificateHandler.get(FILE_TITLE);  // Get the content of the file as a char array
  if (server_certificate) 
  {
    Serial.print("server_certificate content via get(): ");
    Serial.println(server_certificate);
    Device::otaUpdating = true;
    xTaskCreatePinnedToCore (awsOtaTask, "awsOtaTask", 10000, this, 20, NULL, 1);
    //delete[] server_certificate;  
  }
  
}

bool AWS_OTA::startSpiffsUpdate(const char* url) {
  Serial.print("Starting SPIFFS OTA from: ");
  Serial.println(url);

  WiFiClientSecure client;
  client.setInsecure(); // or client.setCACert(server_certificate.c_str())

  HTTPClient https;
  if (https.begin(client, url)) {
    int httpCode = https.GET();
    if (httpCode == HTTP_CODE_OK) {
      int len = https.getSize();
      WiFiClient *stream = https.getStreamPtr();

      if (Update.begin(len, U_SPIFFS)) {
        size_t written = Update.writeStream(*stream);
        if (written == len) {
          Serial.println("SPIFFS OTA complete.");
        } else {
          Serial.printf("SPIFFS OTA incomplete: written %d / %d\n", written, len);
        }

        if (Update.end() && Update.isFinished()) {
          Serial.println("SPIFFS OTA finished successfully.");
          return true;
        } else {
          Serial.printf("SPIFFS OTA error: %u\n", Update.getError());
          return false;
        }
      } else {
        Serial.println("Not enough space for SPIFFS OTA.");
        return false;
      }
    } else {
      Serial.printf("SPIFFS OTA failed, HTTP code: %d\n", httpCode);
      return false;
    }
    https.end();
  } else {
    Serial.println("SPIFFS OTA: unable to connect to server.");
    return false;
  }
}

void awsOtaTask(void* pvParameters) 
{
  AWS_OTA& ota = *AWS_OTA::getInstance();
  ota.setup();
  SmHttp http;
  char ota_Status[10];  

  while (true) 
  {
    Serial.printf("Device::otaUpdating %d\n", Device::otaUpdating);

    if (ota.getStatus()) 
    {
      Serial.print("OTA STATUS: ");
      Serial.println(otastatus);

      if(otastatus == HTTPS_OTA_SUCCESS){
        Serial.println("OTA Success");
        strcpy(ota_Status, OTA_SUCCESS);  // Use strcpy to copy "S" into ota_Status
      }
      else if(otastatus == HTTPS_OTA_FAIL){
        Serial.println("OTA Failed");
        strcpy(ota_Status, OTA_ROLL_BACK);  // Use strcpy to copy "R" into ota_Status
      }
      else{
        Serial.println("OTA Error");
        strcpy(ota_Status, OTA_ROLL_BACK);  // Use strcpy to copy "R" into ota_Status
      }
      break;
      // if (http.notifyOTAUpdate(Device::macAddress)) 
      // {
      //   Serial.println("Device update reported successfully");
      //   strcpy(ota_Status, OTA_SUCCESS);  // Use strcpy to copy "C" into ota_Status
      //   break;
      // } 
      // else 
      // {
      //   Serial.println("Failed to report update to server");
      //   strcpy(ota_Status, OTA_ROLL_BACK);  // Use strcpy to copy "R" into ota_Status
      //   delete[] ota.deviceOtaUrl;
      //   ota.deviceOtaUrl = nullptr;
      //   break;
      // }
    }
    else{
      Serial.print("-*-*OTA STATUS: ");
      Serial.println(otastatus);
      strcpy(ota_Status, OTA_ROLL_BACK);  // Use strcpy to copy "R" into ota_Status
    }

    if (!ota.updated)  // if ota stopped or failed
    {
      if(otastatus == HTTPS_OTA_SUCCESS){
        Serial.println("OTA Success");
        strcpy(ota_Status, OTA_SUCCESS);  // Use strcpy to copy "S" into ota_Status
      }
      else if(otastatus == HTTPS_OTA_FAIL){
        Serial.println("OTA Failed");
        strcpy(ota_Status, OTA_ROLL_BACK);  // Use strcpy to copy "R" into ota_Status
      }
      else{
        Serial.println("OTA Error");
        strcpy(ota_Status, OTA_ROLL_BACK);  // Use strcpy to copy "R" into ota_Status
      }
      break;
    }
    delay(1000);
  }
   // SPIFFS OTA if URL was present
  if (strcmp(ota_Status, OTA_SUCCESS) == 0 && strlen(ota.deviceOtaUrl) > 0) {
    // json_urls = SM_Json::parseDynamic((const char*)pvParameters);
    if (json_urls.spiffs.length()) {
      if(ota.startSpiffsUpdate(json_urls.spiffs.c_str())){
        if (http.notifyOTAUpdate(Device::macAddress)) 
        {
          Serial.println("Device update reported successfully");
          // strcpy(ota_Status, OTA_SUCCESS);  // Use strcpy to copy "C" into ota_Status
        } 
        else 
        {
          Serial.println("Failed to report update to server");
          // strcpy(ota_Status, OTA_ROLL_BACK);  // Use strcpy to copy "R" into ota_Status
          delete[] ota.deviceOtaUrl;
          ota.deviceOtaUrl = nullptr;
        }
      }
    }
  }
  http.updateDeviceStatus(Device::macAddress, ota_Status);
  rgb->currentMode = rgb_last_mode;

  if (strcmp(ota_Status, OTA_SUCCESS) == 0) 
    ESP.restart();

  Device::otaUpdating = false;
  Serial.printf("Device::otaUpdating flag: %d\n", Device::otaUpdating);
  Serial.println("OTA Task Deleted");
  vTaskDelete(NULL);
}
