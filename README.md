# Smart Masjid - Button Firmware
 To develop firmware for IoT Button and IoT Speaker products

## API Swagger Documentation
[Swagger](http://smartmasjid-api-qa.futuriklabs.com/swagger-ui/index.html#/)
#### Bearer Token
eyJhbGciOiJIUzUxMiJ9.eyJzdWIiOiJtdWRhc3Npci5zYWlmaUBnbWFpbC5jb20iLCJ1c2VySWQiOjIsIm1hc2ppZElkIjowLCJ1c2VyVHlwZSI6eyJpZCI6MywibmFtZSI6IlN5c3RlbSBBZG1pbiIsImNhdGVnb3J5IjoiQWRtaW4iLCJwZXJtaXNzaW9ucyI6bnVsbH0sImlhdCI6MTcyNDc5Mzk5MiwiZXhwIjoxNzQwNjA1MTkyfQ.GbKtdr789PhP3dLlLYjFeiZc-LKIcw2wC8lk0g0LKJRb4ETHm13sfRmJF0opESy5cCsRzlBVXMLLUJI7xW0lHA

## Firmware Flowchart
[DrawIO Flowchart](https://app.diagrams.net/#G1D794t6WH5X4qnHniXaRM_4V8VDZjGsNo#%7B%22pageId%22%3A%22XOtjJPAAMMDfJb9Jv2DV%22%7D)

## Testing BLE Provisioning with ESP32 Dev Module
### Required Software
1. [ESP32 Flash Download Tool](https://www.espressif.com/sites/default/files/tools/flash_download_tool_3.9.4.zip)

### Testing procedure
1. Connect the ESP32 Dev Module to the PC via MicroUSB cable.
2. Open flash download tool and select *Chip Type* as *ESP32* and *WorkMode* as *Develop*. Press *OK* to move to the next screen. 
3. Select correct *COM Port* and set *Baud rate* to *921600* and then click erase button on new window that opens, to first completely wipe off the ESP32 memory.
4. Upload the provided binaries as follows:
   * Smart-Speaker.ino.bootloader.bin  OR  Smart-Button.ino.bootloader.bin -> 0x1000
   * Smart-Speaker.ino.partitions.bin  OR  Smart-Button.ino.partitions.bin -> 0x8000
   * Smart-Speaker.ino.bin             OR  Smart-Button.ino.bin            -> 0x10000
5. Press the start button to start the flashing process.
6. Once finished, close the software and reset the ESP32 using EN button on the Dev Module. For the custom board, you can power cycle the board (turning power ON/OFF)
7. Open termite serial terminal and connect to ESP32 serial port at baud rate of 115200 in order to observe console log.
8. The ESP32 will first go into WiFi mode and search its memory for stored WiFi credentials, failing to find any, it will fallback to BLE mode.
9. Supply the WiFi credentials using Smart Masjid App or alternatively nRF Connect app at READ characteristic in JSON format as below:  
 { "ssid":"string", "pass":"string", "masjid_id":"string" }
10. After receiving, the device will initiate connection to internet and after successful connection, it will hit an API to tell the cloud its online and save provided credentials in EEPROM (permanent memory)
11. For another cycle of testing, repeat all the above steps.

## ESP32 Board and Libreries
1. Esp32 Board Version     (Version : 2.0.6)
2. PubSubClient            (Version : 2.8)
3. WiFi                    (Version : 0.0.3)
4. WiFiClientSecure        (Version : 1.0.0)
5. ArduinoJson             (Version : 6.21.2)
6. ESP32-audioI2S-master
7. SPIFFS 
8. FastLED  

## Steps to Add, Build, and Flash an ESP32 Project in Arduino IDE
1. Open Arduino IDE.
2. Go to File > Preferences and add the following URL under Additional Boards Manager [URLs](https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json):
3. Select your ESP32 board and the correct port from Tools.
4. Click the Verify button to compile the code and check for errors. Make sure all the correct libraires are already added as shared above
5. Connect your ESP32 via Voltlink or USB.
6. Click Upload to flash the firmware onto the ESP32.
   Note : Getting Problem to installing Esp32 then go to [this link](https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/)

## Steps to Add a Library in Arduino IDE
1. Go to Sketch > Include Library > Manage Libraries.
2. Use the search bar to find the desired library.
3. Click on the library and then click the Install button.
4. Add the library to your sketch using Sketch > Include Library and select the installed library.

## Steps to Add a ZIP Library in Arduino IDE
1. Obtain the library ZIP file from a trusted source..
2. Open Arduino IDEy.
3. Go to Sketch > Include Library > Add .ZIP Library.
4. Navigate to the downloaded ZIP file and click Open.
5. The library will be installed, and you can include it in your sketch as needed.

## Upload server.crt to SPIFFS
To upload the server.crt file to SPIFFS:

1. Install the ESP32 Filesystem Uploader plugin for Arduino IDE([1.8.x](https://forum.arduino.cc/t/using-the-filesystem-spiffs-with-arduino-ide-2-0-x-is-problematic/1162671)). Follow the instructions in [this guide](https://randomnerdtutorials.com/install-esp32-filesystem-uploader-arduino-ide/).
2. Place the server.crt file inside the data/ folder of your project directory.
3. Open Arduino IDE, then go to partition Scheme select 8MB with spiffs.
4. Then go to Tools > ESP32 Sketch Data Upload to upload the server.crt to the device's SPIFFS. 

## Change in the Audio.h file 
![image](https://github.com/user-attachments/assets/da2e446b-6527-4df9-82ad-ca7283edc6c8)

* Go to the Audio.h located in the ESP32-audioI2S-master folder and serach m_timeout_ms then just multiply the value with 5.(m_timeout_ms = 250 *5;)
* Otherwise  audio library will fail to connect  host(adhan stream url)

## Modify The MQTT_MAX_PACKET_SIZE
![image](https://github.com/user-attachments/assets/d95e996b-85a2-42e5-9f77-c1a1b35effed)

* Go to the PubSubClient.h and serach MQTT_MAX_PACKET_SIZE then update it to 4000.
* Otherwise the Ota Url payload Will not be recieved during Ota Update.
