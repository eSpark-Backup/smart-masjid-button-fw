#include "LED.hpp"
#include <cstring>

LED rgb;
CRGBPalette16 LED::currentPalette;
MODE LED::currentMode = MODE::RGB_OFF;
AdhanStatus LED::ADHAN_FLAG = AdhanStatus::NO_STREAMING;

LED* LED::instance = nullptr;

LED* LED::getInstance()
{
  if (!instance) 
      {
        instance = new LED();
      }
      return instance;
}

void LED::setColor(CRGB color)
{
    currentPalette = CRGBPalette16(color, color, CRGB::Black, CRGB::Black);
    FastLED.setBrightness(BRIGHTNESS);
}

void LED::setup()
{
    FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);
}

void LED::showSolidColor(CRGB color)
{

    FastLED.setBrightness(BRIGHTNESS);
    fill_solid(leds, NUM_LEDS, color);
    FastLED.show();
}


void LED::showBlinkingColor(CRGB color, uint16_t fadeDelay) {
    static uint32_t previousMillis = 0;
    static int brightness = 0;
    static bool increasing = true;

    if (millis() - previousMillis >= fadeDelay) {
        previousMillis = millis();

        // Adjust brightness smoothly
        FastLED.setBrightness(brightness);
        fill_solid(leds, NUM_LEDS, color);
        FastLED.show();

        // Adjust brightness direction
        if (increasing) {
            brightness++;
            if (brightness >= BRIGHTNESS) increasing = false;
        } else {
            brightness--;
            if (brightness <= 0) increasing = true;
        }
    }
}

bool LED::blinkColorNTimes(CRGB color, uint16_t interval, uint8_t times) {
    static uint32_t previousMillis = 0;
    static int brightness = 0;
    static bool increasing = true;
    static uint8_t fadeCount = 0;

    if (fadeCount >= times) {
        // Reset everything after done
        brightness = 0;
        increasing = true;
        fadeCount = 0;

        fill_solid(leds, NUM_LEDS, CRGB::Black);  // Turn LEDs off
        FastLED.show();

        return true; 
    }

    if (millis() - previousMillis >= interval) {
        previousMillis = millis();

        FastLED.setBrightness(brightness);
        fill_solid(leds, NUM_LEDS, color);
        FastLED.show();

        if (increasing) {
            brightness++;
            if (brightness >= BRIGHTNESS) {
                increasing = false;
            }
        } else {
            brightness--;
            if (brightness <= 0) {
                increasing = true;
                fadeCount++;  // one full fade cycle done
            }
        }
    }

    return false;  // still blinking
}

void LED::circleBlinking(CRGB color, const char *speed)
{
    uint8_t brightness = 255;
    uint8_t colorIndex = 1;
    CRGB black = CRGB::Black;
    CRGBPalette16 currentPalette_Black = black;
    // currentPalette = CRGBPalette16(color, color, CRGB::Black,  CRGB::Black);
    setColor(color);

    for (int i = 0; i < NUM_LEDS; ++i)
    {
        // Turn off all LEDs
        fill_solid(leds, NUM_LEDS, CRGB::Black);
        // Turn on the current LED
        leds[i] = ColorFromPalette(currentPalette, colorIndex, brightness, currentBlending);
        FastLED.show();

        // if (speed == "FAST")
        if (strcmp(speed, "FAST") == 0)
            FastLED.delay(10000 * 3 / UPDATES_PER_SECOND);
        // else if (speed == "MEDIUM")
        else if (strcmp(speed, "MEDIUM") == 0)
            FastLED.delay(100000 / UPDATES_PER_SECOND);
        // else if (speed == "SLOW")
        else if (strcmp(speed, "SLOW") == 0)
            FastLED.delay(100000 * 5 / UPDATES_PER_SECOND);
        else
        {
            setColor(black);
        }
    }
}

void LED::turnOffLeds()
{
    FastLED.setBrightness(0);
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
}

void LED::createTask()
{
    xTaskCreatePinnedToCore(rgbLedTask, "rgbLedTask", 6000, this, 0, NULL, 0);
}

void rgbLedTask(void *pvParameters)
{
    // delay(3000);
    rgb.setup();
    rgb.orange = CHSV(HUE_ORANGE, 255, 255);
    rgb.red = CHSV(HUE_RED, 255, 255);
    rgb.green = CHSV(HUE_GREEN, 255, 255);
    rgb.darkgreen = CRGB::DarkGreen;
    rgb.goldenRod = CRGB::DarkGoldenrod;   // hsv(43, 94%, 72%)
    rgb.white = CHSV(0, 0, 255); // White color
    rgb.darkred = CRGB::DarkRed;

    // rgb.turnOffLeds();

    while (1)
    {
        switch (LED::currentMode)
        {
            case MODE::BLE:
                rgb.showBlinkingColor(rgb.goldenRod, 200); // LED behavior is Orange circle blinking at medium speed
                // Serial.println("LED Orange circle blinking\n");
                break;

            case MODE::ON_POWER_UP:
                rgb.showSolidColor(rgb.white); // LED behavior should be solid white
                // Serial.println("LED solid white blinking\n");
                break;

            case MODE::ADHAN_STREAMING:
                rgb.circleBlinking(rgb.green, "SLOW"); // LED behavior should be Green circle blinking slowly
                // Serial.println("LED Green circle blinking slowly\n");
                break;
                
            case MODE::ADHAN_CALLED:
                rgb.showSolidColor(rgb.green); // LED behavior should be green
                // Serial.println("LED green\n");
                break;

            case MODE::ADHAN_RESET_MODE:
                rgb.showSolidColor(rgb.white); // LED behavior should be white
                // Serial.println("LED white\n");
                break;
                
            case MODE::WIFI_CONNECTED:
                rgb.showSolidColor(rgb.darkgreen); // the board gets connected to WiFi but is sitting idle
                // Serial.println("LED darkgreen\n");
                break;

            case MODE::ADHAN_STARTED_INTERNET_STOPPED:
                if(rgb.blinkColorNTimes(rgb.darkred, 200, 3)) // LED should blink dark red 3 times
                {
                    rgb.currentMode = MODE::ADHAN_STREAMING;
                    // Serial.println("LED blink dark red 3 times\n");
                }
                break;

  
            case MODE::OTA_IN_PROGRESS:
                rgb.showBlinkingColor(rgb.white, 200); // LED behavior should be white blinking
                Serial.println("LED white blinking\n");
                break;

            case MODE::BATTRY_IS_CHARGING:
                rgb.showSolidColor(rgb.green); // It should show Green light constantly if USB power is plugged in
                Serial.println("LED green\n");
                break;

            case MODE::LOW_BATTERY:
                rgb.showSolidColor(rgb.red); // LED behavior is Red constant only when there is no USB charger plugged in
                Serial.println("LED red\n");
                break;

            case MODE::WIFI_DISCONNECTED:
                rgb.showSolidColor(rgb.darkred); // LED behavior should be red 
                Serial.println("LED darkred\n");
                break;

            default:
                rgb.turnOffLeds();
                Serial.println("LEDs OFF\n");
                break;
        }
    }

    vTaskDelete(NULL);
}
