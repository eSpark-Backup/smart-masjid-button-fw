# Button Firmware on PlatformIO


## Installation of Platformio in VSCode:
Follow these steps to install PlatformIO in your development environment:

**1. Install Visual Studio Code (VSCode):**
- Download and install Visual Studio Code from https://code.visualstudio.com/.

**2. Install PlatformIO Extension:**
- Open Visual Studio Code.
- Go to the Extensions view by clicking the Extensions icon in the Activity Bar on the side of the VSCode window or pressing Ctrl+Shift+X.
- Search for "PlatformIO IDE" in the Extensions Marketplace.
- Click Install to add the PlatformIO IDE extension to your VSCode.

**3. Verify Installation:**
- After installation, restart Visual Studio Code if prompted.
- Click on the PlatformIO Home icon in the left sidebar to open the PlatformIO home screen.
- Ensure PlatformIO is working by checking that the home screen loads without errors.

## Uploading a File (server.crt) to ESP32 SPIFFS:
Follow these steps to upload the server.crt file to the ESP32 SPIFFS:
1. **Prepare the File:**
   - Place the `server.crt` file inside the `data` directory in your project folder. If the `data` directory does not exist, create it in the root of your project.

2. **Build the Filesystem Image:**
- Open PlatformIO in your IDE (e.g., VSCode).
- Click the **PlatformIO Home** (PIO) icon on the left sidebar to access the project tasks.
- Navigate to the `env:esp32` environment. (The exact name may vary depending on your board configuration.)
- Expand the **Platform** menu under `env:esp32`.
- Select **Build Filesystem Image** to create the SPIFFS image.

3. **Upload the Filesystem Image:**
   - After building the image, select **Upload Filesystem Image** from the same `env:esp32` menu to upload the contents of the `data` folder, including `server.crt`, to the ESP32.
    
![image](https://github.com/user-attachments/assets/9c35eefe-c0db-451b-a193-9c9493225c1e)

4. **Verify Upload:**
   - Uncomment this line and upload to view the uploaded file on the serial monitor and confirm that the file was successfully uploaded to SPIFFS.
     ```
       certificateHandler.setup();
       certificateHandler.print();
     ```
![image](https://github.com/user-attachments/assets/044501c1-d76c-4516-8f90-f87250dd6056)

5. **Result**
 - After uploading to the ESP32, check the result on the serial monitor

![image](https://github.com/user-attachments/assets/04e1a16d-9f66-41a9-aba9-78c6f7af1345)

6- **Demo video link**
[](https://www.loom.com/share/f365b2b5d782472e8970aae6634bd5bf?sid=1906fb31-ed4c-4add-914f-2bdc1071db29)

## ESP32 Board and Libreries:
    - Esp32 Board Version (Version : 2.0.6) 
    - PubSubClient (Version : 2.8)
    - ArduinoJson (Version : 7.2.1)

## Upload Binaries on ESP32:
Follow these steps to upload the binary files to the ESP32 through `flash_download_tool`:
### Required Software
1. [ESP32 Flash Download Tool](https://www.espressif.com/sites/default/files/tools/flash_download_tool_3.9.4.zip)

### Testing procedure
1. Connect the ESP32 Dev Module to the PC via MicroUSB cable.
2. Open flash download tool and select *Chip Type* as *ESP32* and *WorkMode* as *Develop*. Press *OK* to move to the next screen. 
3. Select correct *COM Port* and set *Baud rate* to *115200* and then click erase button on new window that opens, to first completely wipe off the ESP32 memory.
4. Upload the provided binaries as follows:
   ```
      * bootloader.bin  OR  bootloader.bin -> 0x1000
      * partitions.bin  OR  partitions.bin -> 0x8000
      * firmware.bin    OR  firmware.bin   -> 0x10000
      * spiffs.bin      OR  spiffs.bin    -> 0x310000
    ```
5. Press the start button to start the flashing process.
6. Once finished, close the software and reset the ESP32 using EN button on the Dev Module. For the custom board, you can power cycle the board (turning power ON/OFF)
7. Open termite serial terminal and connect to ESP32 serial port at baud rate of 115200 in order to observe console log.
8. The ESP32 will first go into WiFi mode and search its memory for stored WiFi credentials, failing to find any, it will fallback to BLE mode.
9. Supply the WiFi credentials using Smart Masjid App or alternatively nRF Connect app at READ characteristic in JSON format as below:  
 { "ssid":"string", "pass":"string", "masjid_id":"string" }
10. After receiving, the device will initiate connection to internet and after successful connection, it will hit an API to tell the cloud its online and save provided credentials in EEPROM (permanent memory)
11. For another cycle of testing, repeat all the above steps.
12. Demo video link:
    [Upload Binaries Demo Link](https://www.loom.com/share/412dfff911c84de7bd726a3a6ad232f1?sid=ba3c8dc7-31e9-4fb5-bad0-06bdca803d01)



 
