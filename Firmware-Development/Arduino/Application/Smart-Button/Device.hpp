#pragma once

#define SM_MODEL "SBB23V1"
#define SM_FIRMWARE_VER "1.0"
#define BLE_MAC_OFFSET 2
#define BATTERY_HEALTH 90
#define MASJID_ID_TEST 3
#define DEVICE_FACTORY_INFO_NAMESPACE "factoryInfo"
#define DEVICE_WIFI_CREDENTIALS_NAMESPACE "credentials"
#define DEVICE_MEMORY_INFO_PRINT_INTERVAL 5000
#define DELAY_INTERVAL  5000

// Timer Settings
#define TIMER_NO 0
#define TIMER_PRESCALAR_1MHZ 80
#define TIMER_ONE_SECOND_COUNTS 1000000
#define TELEMETRY_UPDATE_TIME 1 * 60 // telemetry update time in seconds

class Device 
{
  private:
    // Variables
    static hw_timer_t * timer;      //H/W timer defining (Pointer to the Structure)
    static bool telemetryTrigger;
    
  public:
    // Constructor
    Device() = delete; // Disable constructor

    // Variables
    static char macAddress[20];
    static bool otaUpdating;       //device updating flag
    // Static member methods
    static void setup(); 
    static void loop();
    static void timerSetup();
    static bool deleteMasjidId();
    static bool deleteWificredentials();
    static bool updateWifiCredential(const char* ssid, const char* pass);
    static void getDeviceMacAddress(char* bleMacAddress);
    static bool checkMacAddress();
    static bool storeMacAddress();
    static void memoryMonitoring();

    // ISR
    static void IRAM_ATTR triggerTelemetryUpdate();
};
