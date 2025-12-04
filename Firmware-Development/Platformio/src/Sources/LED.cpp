#include "LED.hpp"
#include <cstring>

MODE rgb_last_mode = RGB_OFF;

LED* rgb = nullptr;
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
    FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    // FastLED.setMaxRefreshRate(400);
    // FastLED.setDither(0);   // removes flicker
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
            brightness = brightness + 15;
            if (brightness >= BRIGHTNESS) increasing = false;
        } else {
            brightness = brightness - 15;
            if (brightness <= 0) increasing = true;
        }
    }
}

// void LED::showBlinkingColor(CRGB color, uint16_t interval) {
//     // Turn LEDs ON with the given color
//         fill_solid(leds, NUM_LEDS, color);
//         FastLED.show();
//         delay(interval);

//         // Turn LEDs OFF
//         fill_solid(leds, NUM_LEDS, CRGB::Black);
//         FastLED.show();
//         delay(interval);
// }

// bool LED::blinkColorNTimes(CRGB color, uint16_t interval, uint8_t times)
// {
//     if (times == 0) return false;  // no blinks requested

//     for (uint8_t i = 0; i < times; i++)
//     {
//         // Turn LEDs ON with the given color
//         fill_solid(leds, NUM_LEDS, color);
//         FastLED.show();
//         delay(interval);

//         // Turn LEDs OFF
//         fill_solid(leds, NUM_LEDS, CRGB::Black);
//         FastLED.show();
//         delay(interval);
//     }

//     return true; // successfully blinked
// }
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
            brightness = brightness + 10;
            if (brightness >= BRIGHTNESS) {
                increasing = false;
            }
        } else {
            brightness = brightness - 10;
            if (brightness <= 0) {
                increasing = true;
                fadeCount++;  // one full fade cycle done
            }
        }
    }

    return false;  // still blinking
}

// void LED::circleBlinking(CRGB color, const char *speed)
// {
//     uint8_t brightness = 255;
//     uint8_t colorIndex = 1;
//     CRGB black = CRGB::Black;
//     CRGBPalette16 currentPalette_Black = black;
//     // currentPalette = CRGBPalette16(color, color, CRGB::Black,  CRGB::Black);
//     setColor(color);

//     for (int i = 0; i < NUM_LEDS; ++i)
//     {
//         // Turn off all LEDs
//         fill_solid(leds, NUM_LEDS, CRGB::Black);
//         // Turn on the current LED
//         leds[i] = ColorFromPalette(currentPalette, colorIndex, brightness, currentBlending);
//         FastLED.show();

//         // if (speed == "FAST")
//         if (strcmp(speed, "FAST") == 0)
//             FastLED.delay(10000 * 3 / UPDATES_PER_SECOND);
//         // else if (speed == "MEDIUM")
//         else if (strcmp(speed, "MEDIUM") == 0)
//             FastLED.delay(100000 / UPDATES_PER_SECOND);
//         // else if (speed == "SLOW")
//         else if (strcmp(speed, "SLOW") == 0)
//             FastLED.delay(100000 * 5 / UPDATES_PER_SECOND);
//         else
//         {
//             setColor(black);
//         }
//     }
// }

// void LED::circleBlinking(CRGB color, const char *speed)
// {
//     uint8_t brightness = 255;
//     uint8_t colorIndex = 1;
//     CRGB black = CRGB::Black;
//     CRGBPalette16 currentPalette_Black = black;
//     setColor(color);

//     uint16_t delayTime;

//     // Determine delay time based on speed
//     if (strcmp(speed, "FAST") == 0)
//         delayTime = 10000 * 3 / UPDATES_PER_SECOND;
//     else if (strcmp(speed, "MEDIUM") == 0)
//         delayTime = 100000 / UPDATES_PER_SECOND;
//     else if (strcmp(speed, "SLOW") == 0)
//         delayTime = 100000 * 5 / UPDATES_PER_SECOND;
//     else
//         delayTime = 50000 / UPDATES_PER_SECOND;

//     for (int i = 0; i < NUM_LEDS; ++i)
//     {
//         // Turn off all LEDs
//         fill_solid(leds, NUM_LEDS, CRGB::Black);

//         // Fade in
//         for (int b = 0; b <= 255; b += 10)
//         {
//             leds[i] = ColorFromPalette(currentPalette, colorIndex, b, currentBlending);
//             FastLED.show();
//             FastLED.delay(delayTime / 10);
//         }

//         // Hold at full brightness
//         FastLED.delay(delayTime);

//         // Fade out
//         for (int b = 255; b >= 0; b -= 10)
//         {
//             leds[i] = ColorFromPalette(currentPalette, colorIndex, b, currentBlending);
//             FastLED.show();
//             FastLED.delay(delayTime / 10);
//         }
//     }
// }

void LED::circleBlinking(CRGB color, const char *speed)
{
    uint8_t brightness = 255;
    uint8_t colorIndex = 1;
    CRGB black = CRGB::Black;
    CRGBPalette16 currentPalette_Black = black;
    setColor(color);

    uint16_t delayTime;
    uint8_t fade_step = 5;

    // Determine delay time based on speed
    if (strcmp(speed, "FAST") == 0)
    {
        fade_step = 15;
        delayTime = 10;   // shorter delay for faster motion
    }
    else if (strcmp(speed, "MEDIUM") == 0)
    {
        fade_step = 5;
        delayTime = 10;
    }
    else if (strcmp(speed, "SLOW") == 0)
    {
        fade_step = 1;
        delayTime = 20;
    }
    else
        delayTime = 15;

    // main circular fade loop
    for (int i = 0; i < NUM_LEDS; ++i)
    {
        int next = (i + 1) % NUM_LEDS;  // next LED index (wrap around at end)

        // fade transition between i and next
        for (int b = 0; b <= 255; b += fade_step)
        {
            // calculate complementary brightness for the two LEDs
            uint8_t fadeOut = 255 - b;
            uint8_t fadeIn = b;

            // set brightness levels
            fill_solid(leds, NUM_LEDS, CRGB::Black);
            leds[i]   = ColorFromPalette(currentPalette, colorIndex, fadeOut, currentBlending);
            leds[next] = ColorFromPalette(currentPalette, colorIndex, fadeIn, currentBlending);

            FastLED.show();
            FastLED.delay(delayTime);
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
    rgb->setup();
    rgb->orange = CHSV(HUE_ORANGE, 255, 255);
    rgb->red = CHSV(HUE_RED, 255, 255);
    rgb->green = CHSV(HUE_GREEN, 255, 255);
    rgb->darkgreen = CRGB::DarkGreen;
    rgb->goldenRod = CRGB::DarkGoldenrod;   // hsv(43, 94%, 72%)
    rgb->white = CHSV(0, 0, 255); // White color
    rgb->darkred = CRGB::DarkRed;

    // rgb->turnOffLeds();

    while (1)
    {
        switch (LED::currentMode)
        {
            case MODE::BLE:
                rgb->showBlinkingColor(rgb->orange, 50); // LED behavior is Orange circle blinking at medium speed
                break;

            case MODE::ON_POWER_UP:
                rgb->showSolidColor(rgb->white); // LED behavior should be white
                break;

            case MODE::ADHAN_STREAMING:
                rgb->circleBlinking(rgb->green, "MEDIUM"); // LED behavior should be Green circle blinking at medium speed
                break;
                
            case MODE::ADHAN_CALLED:
                rgb->showSolidColor(rgb->green); // LED behavior should be green
                break;

            case MODE::ADHAN_RESET_MODE:
                rgb->showSolidColor(rgb->white); // LED behavior should be white
                break;
                
            case MODE::WIFI_CONNECTED:
                rgb->showSolidColor(rgb->white); // the board gets connected to WiFi but is sitting idle
                break;

            case MODE::ADHAN_STARTED_INTERNET_STOPPED:
                if(rgb->blinkColorNTimes(rgb->darkred, 100, 3)) // LED should blink dark red 3 times
                {
                    rgb->currentMode = MODE::ADHAN_STREAMING;
                }
                break;
            case MODE::ADHAN_NOT_STARTED:
                if(rgb->blinkColorNTimes(rgb->darkred, 50, 3)) // LED should fade blink dark red 3 times
                // if(rgb->blinkColorNTimes(rgb->darkred, 250, 3)) // LED should blink dark red 3 times
                {
                    rgb->currentMode = rgb_last_mode;
                }
                break;

            case MODE::OTA_IN_PROGRESS:
                rgb->showBlinkingColor(rgb->white, 100); // LED behavior should be white
                break;

            case MODE::BATTRY_IS_CHARGING:
                rgb->showSolidColor(rgb->green); // It should show Green light constantly if USB power is plugged in
                break;

            case MODE::LOW_BATTERY:
                rgb->showSolidColor(rgb->red); // LED behavior is Red constant only when there is no USB charger plugged in
                break;

            case MODE::WIFI_DISCONNECTED:
                rgb->showSolidColor(rgb->darkred); // LED behavior should be red 
                break;

            case MODE:: RGB_OFF:
                rgb->turnOffLeds();
                break;
                
            default:
                rgb->turnOffLeds();
                break;
        }
    }

    vTaskDelete(NULL);
}
