#include "led_status.h"
#include "reset_task.h"
#include <Preferences.h>
#include <WiFi.h>
#include "configuration.h"
#include "webserial_logging.h"

static gpio_num_t resetPin;
static uint32_t holdTime = RESET_PIN_HOLD_TIME;
static unsigned long pressStart = 0;
static bool buttonPressed = false;

void IRAM_ATTR resetButtonInterrupt() {
    if (digitalRead(resetPin) == LOW) {  // Button is pressed
        if (!buttonPressed) {
            pressStart = millis();  // Record the start time of the button press
            buttonPressed = true;
        }
    } else {  // Button is released
        if (buttonPressed) {
            unsigned long pressDuration = millis() - pressStart;
            if (pressDuration >= holdTime) {
              loggingEnabled = false;
              delay(100);
                // Button was held long enough to trigger reset
                logPrintln("Button held — resetting preferences and restarting...");
                ledStatus.setResetting();
//                unsigned long startAttempt = millis();
//                while (millis() - startAttempt < 1000) {
//                    ledStatus.update();
//                }
//                ledStatus.off();
                ledStatus.update();

                Preferences prefs;
                prefs.begin("wifiConfig", false);
                prefs.clear();
                prefs.end();

                delay(100);
                ESP.restart();  // Restart the ESP32 after resetting preferences
            }
            buttonPressed = false;  // Reset the button state
        }
    }
}

void setupResetButton() {
    resetPin = (gpio_num_t)RESET_PIN;  // Set the reset pin
    pinMode(resetPin, INPUT_PULLUP);   // Set the reset pin as input with pull-up resistor

    // Attach an interrupt to the reset pin: trigger on both rising and falling edges
    attachInterrupt(digitalPinToInterrupt(resetPin), resetButtonInterrupt, CHANGE);
}
