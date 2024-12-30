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

## ESP32 Board and Libreries:
    - Esp32 Board Version (Version : 2.0.6) 
    - PubSubClient (Version : 2.8)
    - ArduinoJson (Version : 7.2.1)


 
