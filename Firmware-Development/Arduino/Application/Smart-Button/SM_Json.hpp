#pragma once
#include <ArduinoJson.h>
#include <unordered_map>
#include <cstring>

class SM_Json 
{
  public:
    SM_Json() = delete; // Disable constructor
    template <typename... Args>
    static void create(char* json, uint8_t size, Args... args) 
    {
      StaticJsonDocument<256> doc;
  
      // Add values to the document using a fold expression
      (addValueToDoc(doc, args.first, args.second), ...);
  
      // Serialize the JSON document
      serializeJson(doc, json, size);
    }
    static std::string create(const char* key, const char* value) 
    {
        DynamicJsonDocument jsonDoc(64);
        jsonDoc[key] = value;
        
        std::string json;
        serializeJson(jsonDoc, json);
        return json;
    }
    template <typename... Args>
    static bool parse(const char* json, Args... args) 
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
  
  static char* parseDynamic(const char* json) 
    {
        // Create a temporary DynamicJsonDocument to handle parsing
        size_t capacity = JSON_OBJECT_SIZE(10) + strlen(json) + 1; // Adjust size based on expected JSON structure
        DynamicJsonDocument doc(capacity);
        DeserializationError error = deserializeJson(doc, json);
        if (error) 
        {
            Serial.println("Error parsing JSON");
            return nullptr; // Return nullptr in case of error
        }
        const char* url = doc["url"]; //'url' is a key in the JSON object
        if (url == nullptr) 
        {
            Serial.println("URL not found in JSON");
            return nullptr;
        }

        size_t len = strlen(url) + 1;
        char* result = new char[len];
        memcpy(result, url, len);

        return result; 
    }
  private:
    template <typename T>
    static void addValueToDoc(JsonDocument& doc, const char* key, const T& value) 
    {
      doc[key] = value;
    }

    template <typename T, typename... Rest>
    static bool parseValues(JsonDocument& doc, T pair, Rest... rest) 
    {
      JsonObject obj = doc.as<JsonObject>();
    
      if (!obj.containsKey(pair.first)) 
      {
        Serial.print(pair.first);
        Serial.println("key not found in JSON");
        return false;
      }
    
      const char* value = obj[pair.first];
      uint8_t len = strlen(value);
      memcpy(pair.second, value, len + 1);
    
      return parseValues(doc, rest...);
    }

    // Base case of recursion to terminate the recursive function call
    static bool parseValues(JsonDocument&) { return true; }
};
