#include "led_status.h"
#include "configuration.h"
#include "webserial_logging.h"

// Define blink intervals
#define CONNECTING_BLINK_INTERVAL 500  // 500 ms for station connecting state
#define ERROR_BLINK_INTERVAL 300      // 300 ms for error state

LEDStatus::LEDStatus() : lastUpdate(0), state(STATION_CONNECTING), blinkInterval(CONNECTING_BLINK_INTERVAL) {}

void LEDStatus::begin() {
  pinMode(LED_PIN, OUTPUT);  // Initialize the LED pin
  digitalWrite(LED_PIN, LOW);  // Start with the LED off
}

void LEDStatus::setStationConnecting() {
  state = STATION_CONNECTING;
  blinkInterval = CONNECTING_BLINK_INTERVAL;
}

void LEDStatus::setStationConnected() {
  state = STATION_CONNECTED;
  digitalWrite(LED_PIN, HIGH);  // LED stays on for station connected state
}

void LEDStatus::setAPMode() {
  state = AP_MODE;
  digitalWrite(LED_PIN, HIGH);  // LED stays on for AP mode
}

void LEDStatus::setErrorStationConnecting() {
  state = ERROR_STATION_CONNECTING;
  blinkInterval = ERROR_BLINK_INTERVAL;
}

void LEDStatus::off() {
  state = OFF;
  digitalWrite(LED_PIN, LOW);  // Turn off the LED
}

void LEDStatus::setWaterRefilling() {
  state = WATER_REFILLING;
  digitalWrite(LED_PIN, HIGH);  // LED stays on for water refilling state
}

void LEDStatus::setResetting() {
  state = RESETTING;
  digitalWrite(LED_PIN, HIGH);  // LED stays on for resetting state
}

void LEDStatus::update() {
  unsigned long now = millis();  // Get current time in milliseconds

  switch (state) {
    case OFF:
      digitalWrite(LED_PIN, LOW);  // Turn off the LED
      break;

    case STATION_CONNECTING:
    case ERROR_STATION_CONNECTING:
      if (now - lastUpdate >= blinkInterval) {
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));  // Toggle the LED state
        lastUpdate = now;
      }
      break;

    case STATION_CONNECTED:
    case AP_MODE:
    case WATER_REFILLING:
    case RESETTING:
      digitalWrite(LED_PIN, HIGH);  // Keep LED on for these states
      break;
  }
}

LEDStatus ledStatus;  // Declare the global LEDStatus object
