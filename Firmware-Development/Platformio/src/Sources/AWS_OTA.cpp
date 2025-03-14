#include"AWS_OTA.hpp"
#include "SM_Http.hpp"
#include"CertificateHandler.hpp"
#include "Device.hpp"

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
      return false;

    case HTTPS_OTA_UPDATING:
      Serial.println("HTTPS OTA is UPDATING");
      break;

    case HTTPS_OTA_IDLE:
      Serial.println("HTTPS OTA is IDLE");
      break;

    default:
      Serial.println("Unknown OTA Status");
      break;
  }

  return false;
}

void AWS_OTA::startFirmwareUpdate(const char* url)
{
  deviceOtaUrl = url;
  Serial.println(deviceOtaUrl);
  
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

void awsOtaTask(void* pvParameters) 
{
  AWS_OTA& ota = *AWS_OTA::getInstance();
  ota.setup();
  SmHttp http;
  char otaStatus[10];  

  while (true) 
  {
    Serial.printf("Device::otaUpdating %d\n", Device::otaUpdating);

    if (ota.getStatus()) 
    {
      if (http.notifyOTAUpdate(Device::macAddress)) 
      {
        Serial.println("Device update reported successfully");
        strcpy(otaStatus, OTA_SUCCESS);  // Use strcpy to copy "C" into otaStatus
        break;
      } 
      else 
      {
        Serial.println("Failed to report update to server");
        strcpy(otaStatus, OTA_ROLL_BACK);  // Use strcpy to copy "R" into otaStatus
        delete[] ota.deviceOtaUrl;
        ota.deviceOtaUrl = nullptr;
        break;
      }
    }
    if (!ota.updated) 
      break;
    delay(1000);
  }
  http.updateDeviceStatus(Device::macAddress, otaStatus);

  if (strcmp(otaStatus, OTA_SUCCESS) == 0) 
    ESP.restart();

  Device::otaUpdating = false;
  Serial.printf("Device::otaUpdating %d\n", Device::otaUpdating);
  Serial.println("OTA Task Deleted");
  vTaskDelete(NULL);
}
