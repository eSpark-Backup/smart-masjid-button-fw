#pragma once
#include <ArduinoJson.h>
#include <unordered_map>
#include <cstring>

struct Json_Urls
{
  String firmware;
  String spiffs;
  String bootloader;
  String partitions;
  String certificate;
};

extern Json_Urls json_urls;

class SM_Json
{
public:
  SM_Json() = delete; // Disable constructor
  template <typename... Args>
  static void create(char *json, uint8_t size, Args... args)
  {
    StaticJsonDocument<256> doc;

    // Add values to the document using a fold expression
    (addValueToDoc(doc, args.first, args.second), ...);

    // Serialize the JSON document
    serializeJson(doc, json, size);
  }
  static std::string create(const char *key, const char *value)
  {
    DynamicJsonDocument jsonDoc(64);
    jsonDoc[key] = value;

    std::string json;
    serializeJson(jsonDoc, json);
    return json;
  }
  template <typename... Args>
  static bool parse(const char *json, Args... args)
  {
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, json);
    if (error)
    {
      Serial.println("Error parsing JSON");
      return false;
    }

    bool result = parseValues(doc, args...);
    return result;
  }

  static Json_Urls parseDynamic(const char *json)
  {
    DynamicJsonDocument doc(2048);
    Json_Urls urls;

    if (deserializeJson(doc, json))
    {
      Serial.println("Error parsing JSON");
      return urls;
    }

    urls.firmware = doc["firmware"] | "";
    urls.spiffs = doc["spiffs"] | "";
    urls.bootloader = doc["bootloader"] | "";
    urls.partitions = doc["partitions"] | "";
    urls.certificate = doc["certificate"] | "";

    return urls;
  }

  // static char *parseDynamic(const char *json)
  // {
  //   // Create a temporary DynamicJsonDocument to handle parsing
  //   size_t capacity = JSON_OBJECT_SIZE(10) + strlen(json) + 1; // Adjust size based on expected JSON structure
  //   DynamicJsonDocument doc(capacity);
  //   DeserializationError error = deserializeJson(doc, json);
  //   if (error)
  //   {
  //     Serial.println("Error parsing JSON");
  //     return nullptr; // Return nullptr in case of error
  //   }
  //   const char *firmware = doc["firmware"];       //'firmware' is a key in the JSON object
  //   const char *partitions = doc["partitions"];   //'partitions' is a key in the JSON object
  //   const char *bootloader = doc["bootloader"];   //'bootloader' is a key in the JSON object
  //   const char *spiffs = doc["spiffs"];           //'spiffs' is a key in the JSON object
  //   const char *certificate = doc["certificate"]; //'certificate' is a key in the JSON object
  //   if (firmware == nullptr)
  //   {
  //     Serial.println("Firmware URL not found in JSON");
  //     return nullptr;
  //   }

  //   size_t len = strlen(firmware) + 1;
  //   char *result = new char[len];
  //   memcpy(result, firmware, len);

  //   return result;
  // }

private:
  template <typename T>
  static void addValueToDoc(JsonDocument &doc, const char *key, const T &value)
  {
    doc[key] = value;
  }

  template <typename T, typename... Rest>
  static bool parseValues(JsonDocument &doc, T pair, Rest... rest)
  {
    JsonObject obj = doc.as<JsonObject>();

    if (!obj.containsKey(pair.first))
    {
      Serial.print(pair.first);
      Serial.println("key not found in JSON");
      return false;
    }

    // const char* value = obj[pair.first];
    // uint8_t len = strlen(value);
    // memcpy(pair.second, value, len + 1);

    JsonVariant value = obj[pair.first];

    if (value.is<const char *>())
    {
      // Case 1: If the value is a string, copy it
      const char *strValue = value.as<const char *>();
      uint8_t len = strlen(strValue);
      memcpy(pair.second, strValue, len + 1);
      Serial.printf("Key: %s, String Value: %s, Length: %d\n", pair.first, strValue, len);
    }
    else if (value.is<JsonObject>())
    {
      // Case 2: If the value is a JSON object, serialize it and copy
      size_t jsonSize = measureJson(value);
      char jsonBuffer[jsonSize + 1]; // Allocate buffer dynamically
      serializeJson(value, jsonBuffer, sizeof(jsonBuffer));

      memcpy(pair.second, jsonBuffer, jsonSize + 1);
      Serial.printf("Key: %s, JSON Object Size: %d\n", pair.first, jsonSize);
    }

    return parseValues(doc, rest...);
  }

  // Base case of recursion to terminate the recursive function call
  static bool parseValues(JsonDocument &) { return true; }
};
