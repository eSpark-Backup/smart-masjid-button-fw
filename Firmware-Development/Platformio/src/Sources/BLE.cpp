#include <Arduino.h>
#include <ArduinoJson.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "BLE.hpp"
#include "Device.hpp"
#include "SM_Json.hpp"
#include "SM_Nvs.hpp"

IWifiProvisioning* pWifiProv = NULL;
bleStatus BLE::status = BLE_STOPPED;

BLEServer *pServer = NULL;
BLEService *pService = NULL;
BLECharacteristic *pTxCharacteristic;
BLECharacteristic *pDeviceCharacteristic;
BLECharacteristic *pProvCharacteristic;

class MyServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer)
  {
    Serial.println("BLE : device connected");
    char device[70];
    char smModel[24];
    char smFirmware[24];
    const char* json = certificateHandler.get(FILE_JSON);

    SM_Json::parse(json , std::make_pair("device_info", device));
    SM_Json::parse((const char*)(device) , std::make_pair("model", smModel));
    SM_Json::parse((const char*)(device) , std::make_pair("firmware_ver", smFirmware));
    
    char factoryInfoJson[100];
    SM_Json::create(factoryInfoJson, sizeof(factoryInfoJson),
                        std::make_pair("device_model", smModel),
                        std::make_pair("mac_address", Device::macAddress),
                        std::make_pair("firmware_rev", smFirmware));                                         
    Serial.println(factoryInfoJson);
    
    pDeviceCharacteristic->setValue(factoryInfoJson);
    pDeviceCharacteristic->notify();
  };

  void onDisconnect(BLEServer *pServer)
  {
    Serial.println("BLE : device disconnected");
    pServer->startAdvertising(); // restart advertising
  }
};

class ProvCharacteristicsCallbacks : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    std::string rxValue = pCharacteristic->getValue();

    if (rxValue.length() > 0)
    {
      char masjidId[32];
      if (SM_Json::parse(rxValue.c_str(), std::make_pair("masjid_id", masjidId))) 
      {
        Serial.printf("Masjid id : %s\n", masjidId);
        // Store masjid id in NVS
          Sm_Nvs::put(DEVICE_FACTORY_INFO_NAMESPACE,
                      std::make_pair("masjidId", masjidId));
      }
      else
      {
        Serial.println("Unable to parse masjis id");
      }  
      
      char ssid[32];
      char pass[32];
      if (SM_Json::parse(rxValue.c_str(), std::make_pair("ssid", ssid), std::make_pair("pass", pass))) 
      {
        if (pWifiProv->connectWifi(ssid, pass))
        {    
          // Store credentials in NVS
          if (Sm_Nvs::put(DEVICE_WIFI_CREDENTIALS_NAMESPACE,
                      std::make_pair("ssid", ssid),
                      std::make_pair("pass", pass)))
          {
            Serial.println("Wifi credentials stored");
          }
        }
      } 
      else
      {
        Serial.println("Unable to parse wifi credentials");
      }     
    }
  }
};

void BLE::setup(IWifiProvisioning* wifiProv)
{
  pWifiProv = wifiProv;
}

bool BLE::initialize()
{
  // Create the BLE Device
  BLEDevice::init("SMBTN_SBB23V1");
  Device::setup();
  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  pService = pServer->createService(SERVICE_UUID);

  //Setting macAdress on BLE Advertisement
  BLEAdvertisementData adv;
  BLEAdvertising* pAdvertising = pServer->getAdvertising();
  adv.setFlags(0x06); 
  adv.setManufacturerData(Device::macAddress);
  pAdvertising->setAdvertisementData(adv);
  pAdvertising->start();

  // Create a BLE Characteristic
  pTxCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID_TX,
      BLECharacteristic::PROPERTY_NOTIFY);

  pTxCharacteristic->addDescriptor(new BLE2902());

  pDeviceCharacteristic = pService->createCharacteristic(
                      DEVICE_CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  pProvCharacteristic = pService->createCharacteristic(
      PROV_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_WRITE);

  pProvCharacteristic->setCallbacks(new ProvCharacteristicsCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
  LED& rgb = *LED::getInstance();
  rgb.currentMode = MODE::BLE;
  status = BLE_STARTED;
  return true;
}

void BLE::stop(void)
{
  // Delete BLE server
  BLEDevice::deinit(true);
  Serial.println("BLE Stopped"); 
  LED& rgb = *LED::getInstance();
  rgb.currentMode = MODE::WIFI_CONNECTED; 
  WifiProvisioning::wifiServiceStarted = true;
  status = BLE_STOPPED;
}

bool BLE::isStarted(void)
{
  return (status == BLE_STARTED);
}
