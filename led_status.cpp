// LEDStatusManager.cpp
#include "led_status.h"

LEDStatus::LEDStatus() : strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800),
  lastUpdate(0), brightness(0), fadingUp(true), state(STATION_CONNECTING) {}

void LEDStatus::begin() {
  strip.begin();
  strip.show();
}

void LEDStatus::setStationConnecting() {
  brightness = 0;
  state = STATION_CONNECTING;
}

void LEDStatus::setStationConnected() {
  brightness = 0;
  state = STATION_CONNECTED;
}

void LEDStatus::setAPMode() {
  brightness = 0;
  state = AP_MODE;
}

void LEDStatus::setErrorStationConnecting() {
  brightness = 0;
  state = ERROR_STATION_CONNECTING;
}

void LEDStatus::off(){
  brightness = 0;
  state = OFF;
}

void LEDStatus::setWaterRefilling() {
  brightness = 0;
  state = WATER_REFILLING;
}

void LEDStatus::setResetting() {
  brightness = 0;
  state = RESETTING;
}


void LEDStatus::update() {
  unsigned long now = millis();
  if(state == OFF) {
    showColor(0, 0, 0); 
  } else if (state == STATION_CONNECTING && now - lastUpdate >= 30) {
    brightness += fadingUp ? 10 : -10;
    if (brightness >= 255) { brightness = 255; fadingUp = false; }
    if (brightness <= 0)   { brightness = 0; fadingUp = true;  }
    showColor(0, brightness/3*2, brightness); // water green pulse
    lastUpdate = now;
  } else if (state == STATION_CONNECTED && now - lastUpdate >= 25) {
    brightness += fadingUp ? 10 : -10;
    if (brightness >= 255) { brightness = 255; fadingUp = false; }
    if (brightness <= 0)   { brightness = 0; fadingUp = true;  }
    showColor(0, brightness, 0); // water green pulse
    lastUpdate = now;
  } else if (state == AP_MODE && now - lastUpdate >= 10) {
    brightness += fadingUp ? 10 : -10;
    if (brightness >= 255) { brightness = 255; fadingUp = false; }
    if (brightness <= 0)   { brightness = 0; fadingUp = true;  }
    showColor(brightness, brightness, 0); // yellow fade
    lastUpdate = now;
  } else if (state == ERROR_STATION_CONNECTING && now - lastUpdate >= 15) {
    brightness += fadingUp ? 10 : -10;
    if (brightness >= 255) { brightness = 255; fadingUp = false; }
    if (brightness <= 0)   { brightness = 0; fadingUp = true;  }
    showColor(brightness, 0, 0); // red fade
    lastUpdate = now;
  } else if (state == WATER_REFILLING) {
    showColor(0, 0, 255); // blue 
    lastUpdate = now;
  } else if (state == RESETTING) {
    showColor(180, 0, 255); // violet 
    lastUpdate = now;
  }
}

void LEDStatus::showColor(uint8_t r, uint8_t g, uint8_t b) {
  strip.setPixelColor(0, strip.Color(r, g, b));
  strip.show();
}

LEDStatus ledStatus;
