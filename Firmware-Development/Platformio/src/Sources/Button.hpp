
// #include "LED.hpp"
#pragma once

#define SMBTN_GPIO 26
#define RESET_GPIO 32

// extern MODE rgb_last_mode;
extern bool BLE_mode;

void check_reset_button();

enum buttonState
{
  UNPRESSED = 0,
  ADHAN_START_REQUESTED,
  ADHAN_STARTED,
  ADHAN_STOP_REQUESTED,
  ADHAN_STOPPED,
  ADHAN__CALLED,
  ADHAN_RESET = 10
};

#define TO_MS(seconds) ((seconds) * 1000)
#define ADHAN_MAX_DURATION TO_MS(5*60)

class Button 
{
  private:
    uint8_t pin;
    static bool resetFlag; 
    static volatile int counter; 
    static uint32_t adhanStartTime;
    volatile buttonState state;
    void interruptHandler();
    
  public:
    Button(uint8_t pin) : pin(pin) {}
    void setup(); 
    void loop();
    
  };
  
extern Button button;
