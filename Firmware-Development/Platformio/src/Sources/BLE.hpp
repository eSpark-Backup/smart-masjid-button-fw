#pragma once
#include "WifiProvisioning.hpp"
#include "CertificateHandler.hpp"
#include "LED.hpp"

#define SERVICE_UUID                "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_TX      "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
#define DEVICE_CHARACTERISTIC_UUID  "6E400010-B5A3-F393-E0A9-E50E24DCCA9E"
#define PROV_CHARACTERISTIC_UUID  "6E400011-B5A3-F393-E0A9-E50E24DCCA9E"

typedef enum 
{
  BLE_STOPPED,
  BLE_STARTED,
} bleStatus;

class BLE
{
  private:
    static bleStatus status ;
    
  public:
    static void setup(IWifiProvisioning* wifiProv);
    static bool initialize();
    static void stop(void);
    static void start(void);
    static bool isStarted(void);
};
