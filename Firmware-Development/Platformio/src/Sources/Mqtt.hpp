#pragma once
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WifiProvisioning.hpp"
#include "Device.hpp"

#define TEXTIFY(A) #A
#define MQTT(State)    "MQTT state : " TEXTIFY(State)
#define MQTT_RECONNECT_DELAY 10
void mqttTask(void* pvParameters);

namespace MQTT
{
  typedef enum
  {
    CONNECTION_TIMEOUT = -4,
    CONNECTION_LOST,
    CONNECTION_FAILED,
    DISCONNECTED,
    CONNECTED,
    CONNECT_BAD_PROTOCOL,
    CONNECT_BAD_CLIENT_ID,
    CONNECT_UNAVAILIABLE,
    CONNECT_BAD_CREDENTIALS,
    CONNECT_UNAUTHORIZED,
  } State;
}

typedef enum {
  DEVICE_RESET,
  DEVICE_WIFI,
  DEVICE_PING,
  DEVICE_OTA_UPDATE,
  UNKNOWN_TOPIC = 0xFF,
} MqttTopic;

typedef enum {
  SUBSCRIBE,
  UNSUBSCRIBE,
} Subscription;

class MQTTClient 
{
private:
  IWifiProvisioning* pWifiProv;
  // MQTT broker configuration
  // const char* mqttServer = "smartmasjid-broker-dev.futuriklabs.com";
  const int mqttPort = 1883;
  // const char* mqttUsername = "device";
  // const char* mqttPassword = "device";

  /* MQTT topic details */
  const char* topicPrefix = "device";

  // MQTT topic to subscribe to
  const char* mqttTopicReset = "reset";
  const char* mqttTopicWifi = "wifi";
  const char* mqttTopicPing  = "ping";
  const char* mqttTopicOta  = "update";


public:
  bool otaUpdating = false;
  MQTTClient(IWifiProvisioning* wifiProv): pWifiProv(wifiProv) {}
  bool setup();
  bool connectToBroker();
  bool connectToBroker(int retries);
  bool subscribeTopics();
  bool unsubscribeTopics();
  void receiveCallback(char* topic, byte* payload, unsigned int length);
  void loop();
  bool subscribe(const char* topic);
  bool unsubscribe(const char* topic);
  MqttTopic getTopic(const char* topic);
  bool extractTopicAndControlSubscription(Subscription control, const char* message);
  bool extractTopicAndSubscribe(const char* message);
  bool extractTopicAndUnsubscribe(const char* message);
  void getTopicPath(char* dest, const char* topicName);
  bool compareTopic(const char* topic, const char* topicName);
  bool controlSubscription(Subscription control, const char* topic);
  void mqttCallback();
  void printState();
  void createTask();
};
