// led_status.h
#ifndef LED_STATUS_H
#define LED_STATUS_H

#include <Arduino.h>


// Define possible states for the LED
enum LEDState {
  OFF,
  STATION_CONNECTING,
  STATION_CONNECTED,
  AP_MODE,
  ERROR_STATION_CONNECTING,
  WATER_REFILLING,
  RESETTING
};

class LEDStatus {
public:
  LEDStatus();  // Constructor
  
  void begin();  // Initialize the LED
  void setStationConnecting();  // Set LED to Station Connecting state
  void setStationConnected();  // Set LED to Station Connected state
  void setAPMode();  // Set LED to AP Mode state
  void setErrorStationConnecting();  // Set LED to Error Station Connecting state
  void off();  // Turn off the LED
  void setWaterRefilling();  // Set LED to Water Refilling state
  void setResetting();  // Set LED to Resetting state
  void update();  // Update the LED state
  
private:
  void blinkLED(unsigned long interval);  // Blink the LED at a specific interval
  unsigned long lastUpdate;  // Last time the LED state was updated
  unsigned long blinkInterval;  // Interval for LED blinking (milliseconds)
  LEDState state;  // Current state of the LED
};

extern LEDStatus ledStatus;  // Declare the global LEDStatus object

#endif  // LED_STATUS_H
