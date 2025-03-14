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
  WIFI_CONNECTED,
  WIFI_DISCONNECTED,
  ADHAN_STREAMING, //2
  BATTRY_IS_CHARGING,//3
  LOW_BATTERY,//4
  RGB_OFF,//5
  ON_POWER_UP//6
} MODE;

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
    void setColor(CRGB color);
    void circleBlinking(CRGB color, const char* speed); // speed can be FAST , MEDIUM and SLOW ,any Argument other than these arguments , the led will not lit
    void turnOffLeds();
    void createTask();
};
