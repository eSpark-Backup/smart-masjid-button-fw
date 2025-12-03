#define MQTT_MAX_PACKET_SIZE 9000

#include <ArduinoJson.h>
#include "Mqtt.hpp"
#include "WifiProvisioning.hpp"
#include "CertificateHandler.hpp"
#include "SM_Json.hpp"
#include "SM_Nvs.hpp"
#include "SM_Http.hpp"
#include"AWS_OTA.hpp"
#include "Button.hpp"
#include "LED.hpp"
#include "OtelLogger.hpp"

bool ping = false;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

char mqttObj[120];
char mqttServer[40];
char mqttUsername[8];
char mqttPassword[8];

void loadConfigfromfile() {
  const char* json = certificateHandler.get(FILE_JSON);
  SM_Json::parse(json , std::make_pair("mqtt", mqttObj));
  SM_Json::parse((const char*)(mqttObj) , std::make_pair("broker", mqttServer));
  SM_Json::parse((const char*)(mqttObj) , std::make_pair("user_name", mqttUsername));
  SM_Json::parse((const char*)(mqttObj) , std::make_pair("password", mqttPassword));
}

bool MQTTClient::setup() 
{
  if (connectToBroker(5))
  {
    LED& rgb = *LED::getInstance();
    rgb.currentMode = MODE::WIFI_CONNECTED;
    return true; 
  }

  return false;
}

bool MQTTClient::connectToBroker(int retries) 
{
  // while (retries--) 
  while (1) // infinite retries
  {
    if (connectToBroker() && subscribeTopics())
    { 
      return true;
    }
    check_reset_button();
    delay(MQTT_RECONNECT_DELAY * 1000);
  }

  return false;
}

bool MQTTClient::connectToBroker() 
{
  loadConfigfromfile();
  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setBufferSize(MQTT_MAX_PACKET_SIZE);
  static std::string clientId = std::to_string(random(1000000000));
  Serial.print("Client id : ");
  Serial.println(clientId.c_str());

  if (!mqttClient.connect(clientId.c_str(), mqttUsername, mqttPassword)) 
  {
    Serial.println("Failed to connect to MQTT broker");
    return false;
  } 

  Serial.println("Connected to MQTT broker");
  return true;
}

bool MQTTClient::subscribeTopics() 
{
  mqttClient.setCallback(std::bind(&MQTTClient::receiveCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

  char topicPath[50];

  const char* topicNames[] = 
  {
    mqttTopicReset,
    mqttTopicWifi,
    mqttTopicPing,
    mqttTopicOta,
    mqttTopicdebug
  };
  
  for (const char* topic : topicNames) 
  {
    getTopicPath(topicPath, topic);
    if (!subscribe(topicPath)) 
    {
      return false;
    }
  }

  return true;
}

void MQTTClient::receiveCallback(char* topic, byte* payload, unsigned int length) 
{
  Serial.print("Received MQTT message [");
  Serial.print(topic);
  Serial.print("]: ");

  Serial.write(payload, length);
  Serial.println();

  switch (getTopic(topic)) 
  {
    case DEVICE_RESET:
      Serial.println("RESET TOPIC");
      OtelLogger::log("INFO", Device::macAddress, "Mqtt message received on RESET Topic. Device is Resetting!");
      unsubscribeTopics();
      Device::deleteMasjidId();
      Device::deleteWificredentials();
      ESP.restart();
      break;

    case DEVICE_WIFI:
    {
      Serial.println("Wifi TOPIC");
      OtelLogger::log("INFO", Device::macAddress, "Mqtt message received on Wifi Topic");
      char ssid[35];
      char pass[35];
      SM_Json::parse((const char*)payload, std::make_pair("ssid", ssid), std::make_pair("password", pass));
      Device::updateWifiCredential(ssid, pass);
      if (pWifiProv->connectWifi(ssid, pass, 10)) 
      {
        setup();
      } 
      else 
      {
        Serial.println("Restart ESP.....");
        ESP.restart();
      }
      break;
    }

    case DEVICE_PING:
    {
      OtelLogger::log("INFO", Device::macAddress, "Mqtt message received on PING Topic. Device will send Telemetry!!");
      SmHttp smHttp;
      ping = true;
      smHttp.sendDeviceTelemetry(Device::macAddress);
      break;
    }
    
    case DEVICE_OTA_UPDATE:
    {
      AWS_OTA& ota = *AWS_OTA::getInstance();
      json_urls  = SM_Json::parseDynamic((const char*)payload);
      if(json_urls.firmware != NULL){
        OtelLogger::log("INFO", Device::macAddress, "Device OTA update starts");
        ota.startFirmwareUpdate(json_urls.firmware.c_str());
      }
      break;
    }

    case DEVICE_DEBUG:
    {
      /* need to implement */
      Serial.println("DEBUG TOPIC");

      // Expect payload "ON" or "OFF"
      char state[8] = {0};
      strncpy(state, (char*)payload, min(length, sizeof(state) - 1));

      if (strcasecmp(state, "ON") == 0) {
          // OtelLogger::enable(true);
          OtelLogger::enabled = true;
          OtelLogger::log("INFO", Device::macAddress, "Debug Logging Enabled");
      } else if (strcasecmp(state, "OFF") == 0) {
          OtelLogger::enabled = false;
          OtelLogger::log("INFO", Device::macAddress, "Debug Logging Disabled");
          // OtelLogger::enable(false);
        } else {
          Serial.println("Invalid Mqtt message received on topic debug");
          OtelLogger::log("ERROR", Device::macAddress, "Invalid Message Received on debug Topic ");
      }
      break;
    }


    default:
      Serial.println("UNKNOWN TOPIC");
      break;
  }
}

void MQTTClient::loop() 
{
    if (!mqttClient.connected() && WiFi.status() == WL_CONNECTED) mqttCallback(); 
      mqttClient.loop();
}

bool MQTTClient::subscribe(const char* topic) 
{
  if (mqttClient.subscribe(topic)) 
  {
    Serial.printf("Subscribed to %s\n", topic);
    OtelLogger::log("INFO", Device::macAddress, "MQTT Topics Subscribed");
    return true;
  }
  return false;
}

bool MQTTClient::unsubscribe(const char* topic) 
{
  if (mqttClient.unsubscribe(topic)) 
  {
    Serial.printf("Unsubscribed to %s\n", topic);
    OtelLogger::log("ERROR", Device::macAddress, "MQTT Topics Unsubscribed");
    return true;
  }
  return false;
}

MqttTopic MQTTClient::getTopic(const char* topic) 
{
  if (compareTopic(topic, mqttTopicReset)) 
  {
    return DEVICE_RESET;
  }

  else if (compareTopic(topic, mqttTopicWifi)) 
  {
    return DEVICE_WIFI;
  }

  else if (compareTopic(topic, mqttTopicPing)) 
  {
    return DEVICE_PING;
  }
  else if (compareTopic(topic, mqttTopicOta)) 
  {
    return DEVICE_OTA_UPDATE;
  }
  else if (compareTopic(topic, mqttTopicdebug)){
    return DEVICE_DEBUG;
  }
  return UNKNOWN_TOPIC;
}

bool MQTTClient::compareTopic(const char* topic, const char* topicName) 
{
  char topicPath[50];
  getTopicPath(topicPath, topicName);
  return !strcmp(topicPath, topic);
}

bool MQTTClient::extractTopicAndSubscribe(const char* message) 
{
  return extractTopicAndControlSubscription(SUBSCRIBE, message);
}

bool MQTTClient::extractTopicAndUnsubscribe(const char* message) 
{
  return extractTopicAndControlSubscription(UNSUBSCRIBE, message);
}

bool MQTTClient::extractTopicAndControlSubscription(Subscription control, const char* message) 
{
  bool ret = false;
  char topic[64];

  if (SM_Json::parse(message, std::make_pair("topic", topic))) 
  {
    ret = controlSubscription(control, topic);
  } 
  else 
  {
    Serial.println("Unable to parse topic");
    OtelLogger::log("ERROR", Device::macAddress, "Unable to parse MQTT Topic");
  }
  return ret;
}

bool MQTTClient::controlSubscription(Subscription control, const char* topic) 
{
  bool ret = false;

  switch (control) 
  {
    case SUBSCRIBE:
      ret = subscribe(topic);
      break;

    case UNSUBSCRIBE:
      ret = unsubscribe(topic);
      break;

    default:
      Serial.println("UNKNOWN CONTROL");
  }

  return ret;
}

void MQTTClient::getTopicPath(char* dest, const char* topicName) 
{
  // Copy the prefix to the destination buffer
  strcpy(dest, topicPrefix);

  // Add the MAC address to the destination buffer, separating it with slashes
  strcat(dest, "/");
  strcat(dest, Device::macAddress);

  // Add the topic name to the destination buffer, also separated with slashes
  strcat(dest, "/");
  strcat(dest, topicName);
}

bool MQTTClient::unsubscribeTopics() 
{
  char topicPath[50];
  
  const char* topicNames[] = 
  {
    mqttTopicReset,
    mqttTopicWifi,
    mqttTopicPing,
    mqttTopicOta,
    mqttTopicdebug  
  };

  for (const char* topic : topicNames) 
  {
    getTopicPath(topicPath, topic);
    if (!unsubscribe(topicPath)) 
    {
      return false;
    }
  }
  
  return true;
}

// void MQTTClient::mqttCallback() 
// {
//     Serial.println("MQTT client not connected. Reconnecting...");
//     printState();
//     Serial.print("\nWIFI RSSI : ");
//     Serial.println(WiFi.RSSI());
//     connectToBroker(10);
// }

void MQTTClient::mqttCallback() 
{
      Serial.println("MQTT client not connected. Reconnecting...");
      OtelLogger::log("ERROR", Device::macAddress, "MQTT client not connected. Reconnecting...");
      printState();
      Serial.print("WIFI RSSI : ");
      Serial.println(WiFi.RSSI());

      // int retryCount = 0;
      // const int maxRetries = MAX_WIFI_RETRIES;

        // while (WiFi.status() != WL_CONNECTED && retryCount < maxRetries) {
        //   Serial.print("WiFi disconnected. Retrying (");
        //   Serial.print(retryCount + 1);
        //   Serial.println("/10)...");
        //   WiFi.disconnect();
        //   WifiProvisioning::wifiServiceStarted = false;
        //   delay(1000);
        //   WiFi.reconnect();
        //   delay(2000); 
        //   retryCount++;
        // }
        if (WiFi.status() != WL_CONNECTED){
          // Serial.println("Failed to reconnect after 10 attempts. Entering sleep mode...");
          // OtelLogger::log("ERROR", Device::macAddress, "Failed to reconnect to after 10 attempts. Entering sleep mode for 5 minutes");
          // Serial.println("Enter Sleep mode for 5 minutes.");
          // delay(10);
          // esp_sleep_enable_timer_wakeup(SLEEP_DURATION * 60 * 1000000); // 5 minute in microseconds
          // esp_light_sleep_start();
          Serial.println("WiFi not connected.  Returning from MQTT callback.");
          return;
        }
        else {
          Serial.println("WiFi Connected, Attempting to MQTT Connection...");
          if(connectToBroker(1)) // ignore 1, infinite retries 
          {
            LED& rgb = *LED::getInstance();
            rgb.currentMode = MODE::WIFI_CONNECTED;
            WifiProvisioning::wifiServiceStarted = true;
            Serial.println("Wifi and MQTT connection established successfully");
          } 
          else 
          {
            WifiProvisioning::wifiServiceStarted = false;
            Serial.println("Failed to established MQTT connection....");
            // Serial.println("Failed to reconnect to MQTT after 5 attempts. Entering sleep mode...");
            // Serial.println("Sleep mode for 5 minutes.");
            delay(10);
            // esp_sleep_enable_timer_wakeup(SLEEP_DURATION * 60 * 1000000); // 5 minute in microseconds
            // esp_light_sleep_start();
            // Serial.println("Woke up from sleep after 5 minutes");
          }
        }
}

void MQTTClient::printState()
{
  int state = mqttClient.state();
  switch(state)
  {
    case MQTT::CONNECTION_TIMEOUT:
      printf("%s\n", MQTT(CONNECTION_TIMEOUT));
      break;

    case MQTT::CONNECTION_LOST:
      printf("%s\n", MQTT(CONNECTION_LOST));
      break;

    case MQTT::CONNECTION_FAILED:
      printf("%s\n", MQTT(CONNECTION_FAILED));
      break;

    case MQTT::DISCONNECTED:
      printf("%s\n", MQTT(DISCONNECTED));
      break;

    case MQTT::CONNECTED:
      printf("%s\n", MQTT(CONNECTED));
      break;

    case MQTT::CONNECT_BAD_PROTOCOL:
      printf("%s\n", MQTT(CONNECT_BAD_PROTOCOL));
      break;

    case MQTT::CONNECT_BAD_CLIENT_ID:
      printf("%s\n", MQTT(CONNECT_BAD_CLIENT_ID));
      break;

    case MQTT::CONNECT_UNAVAILIABLE:
      printf("%s\n", MQTT(CONNECT_UNAVAILIABLE));
      break;

    case MQTT::CONNECT_BAD_CREDENTIALS:
      printf("%s\n", MQTT(CONNECT_BAD_CREDENTIALS));
      break;

    case MQTT::CONNECT_UNAUTHORIZED:
      printf("%s\n", MQTT(CONNECT_UNAUTHORIZED));
      break;
  }
}

void MQTTClient::createTask()
{
  xTaskCreatePinnedToCore (mqttTask, "mqttTask", 10000, this, 0, NULL, 0);
}

void mqttTask(void* pvParameters) 
{
  MQTTClient* mqtt = (MQTTClient*) pvParameters;
  mqtt->setup();

  while(1)
  {
    // if(!BLE_mode){
      mqtt->loop();
      if(mqtt->otaUpdating)
      {
        break;
      }
    // }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }

  vTaskDelete( NULL );
}
