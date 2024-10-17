#pragma once
#include <Preferences.h>

class Sm_Nvs 
{
  public:
    Sm_Nvs() = delete; // Disable constructor
    
    template <typename... Args>
    static bool get(const char* nvsNamespace, Args... pair) 
    {
      Preferences preferences;
      preferences.begin(nvsNamespace, false);
      bool success = (getVal(preferences, pair.first, pair.second) && ...);
      preferences.end();
      return success;
    }
    
    template <typename... Args>
    static bool put(const char* nvsNamespace, Args... pair) 
    {
      Preferences preferences;
      preferences.begin(nvsNamespace, false);
      bool success = (putVal(preferences, pair.first, pair.second) && ...);
      preferences.end();
      return success;
    }
    
    template <typename... Args>
    static bool del(const char* nvsNamespace, Args... keys) 
    {
      Preferences preferences;
      preferences.begin(nvsNamespace, false);
      bool success = (delVal(preferences, keys) && ...);
      preferences.end();
      return success;
    }
  
  private:
    static bool getVal(Preferences& preferences, const char* key, char* result) 
    {
      String value = preferences.getString(key, "");
      if (!value.isEmpty()) 
      {
        strcpy(result, value.c_str());
        return true;
      }
      return false;
    }
    
    static bool putVal(Preferences& preferences, const char* key, const char* value) 
    {
        return preferences.putString(key, value);
    }

    static bool delVal(Preferences& preferences, const char* key) 
    {
        return preferences.remove(key);
    }
};
