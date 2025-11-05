#pragma once
#include<Arduino.h>
#include <FastLED.h>

#define DATA_PIN     25 // DATA_PIN
#define NUM_LEDS    6
#define BRIGHTNESS 100
#define LED_TYPE    TM1803 // WS2812B is not functioning 
#define COLOR_ORDER GRB
#define UPDATES_PER_SECOND 1000
#define FADE_DURATION 10

typedef enum MODE 
{ 
  BLE, //1
  WIFI_CONNECTED, //2
  WIFI_DISCONNECTED, //3
  ADHAN_STREAMING, //4
  BATTRY_IS_CHARGING,//5
  LOW_BATTERY,//6
  RGB_OFF,//7
  ON_POWER_UP,//8
  ADHAN_CALLED, //9
  ADHAN_RESET_MODE, //10
  ADHAN_STARTED_INTERNET_STOPPED, //11
  OTA_IN_PROGRESS, //12
  ADHAN_NOT_STARTED //13
} MODE;


extern MODE rgb_last_mode;

typedef enum AdhanStatus 
{ 
  STREAMING,
  NO_STREAMING
} AdhanStatus;

void rgbLedTask(void* pvParameters);

class LED
{
  private:
    static CRGBPalette16 currentPalette;
    TBlendType currentBlending;

  public:
    CRGB orange;
    CRGB green;
    CRGB red;
    CRGB darkgreen;
    CRGB goldenRod;
    CRGB darkred;
    CRGB white;
    CRGB leds[NUM_LEDS];
    static MODE currentMode;
    static AdhanStatus ADHAN_FLAG;
    
    LED() {}
    LED(const LED&) = delete;
    LED& operator=(const LED&) = delete;
    static LED* getInstance(); 
    static LED* instance;

    void setup();
    void showSolidColor(CRGB color);
    void showBlinkingColor(CRGB color, uint16_t fadeDelay);
    bool blinkColorNTimes(CRGB color, uint16_t interval, uint8_t times);
    void setColor(CRGB color);
    void circleBlinking(CRGB color, const char* speed); // speed can be FAST , MEDIUM and SLOW ,any Argument other than these arguments , the led will not lit
    void turnOffLeds();
    void createTask();
};

extern LED rgb;
