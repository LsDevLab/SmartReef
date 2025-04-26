// LEDStatusManager.h
#pragma once
#include <Adafruit_NeoPixel.h>

#define LED_PIN 48
#define NUM_LEDS 1

class LEDStatus {
public:
  LEDStatus();
  void begin();
  void update();

  void off();
  void setStationConnecting(); 
  void setStationConnected();  
  void setAPMode();     
  void setWaterRefilling();    
  void setResetting();        
       

  // errors
  void setErrorStationConnecting();  

private:
  enum State { STATION_CONNECTING, STATION_CONNECTED, AP_MODE, ERROR_STATION_CONNECTING, OFF, WATER_REFILLING, RESETTING } state;
  Adafruit_NeoPixel strip;
  unsigned long lastUpdate;
  int brightness;
  bool fadingUp;

  void showColor(uint8_t r, uint8_t g, uint8_t b);
};

extern LEDStatus ledStatus;
