#include <Arduino.h>
#include "LED.hpp"
#include "WifiProvisioning.hpp"
#include "Button.hpp"


class WifiMonitoring{
    public:
        void MonitorWifi(){
            if(!WiFi.isConnected() || WifiProvisioning::wifiServiceStarted == false)
            { 
                if(!BLE_mode){
                    LED& rgb = *LED::getInstance();
                    rgb.currentMode = MODE::WIFI_DISCONNECTED;
                }
            }
            else
            {
                LED& rgb = *LED::getInstance();
                // rgb.currentMode = MODE::WIFI_CONNECTED;
            }
        }
};