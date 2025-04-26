#include "reset_task.h"
#include "led_status.h"
#include <Preferences.h>

static gpio_num_t resetPin;
static uint32_t holdTime;
TaskHandle_t resetTaskHandle;

void resetButtonTask(void *parameter) {
    pinMode(resetPin, INPUT_PULLUP);

    bool wasPressed = false;
    unsigned long pressStart = 0;

    while (true) {
        bool pressed = digitalRead(resetPin) == LOW;

        if (pressed && !wasPressed) {
            pressStart = millis();
            wasPressed = true;
        }

        if (!pressed && wasPressed) {
            wasPressed = false;
        }

        if (pressed && (millis() - pressStart >= holdTime)) {
            Serial.println("Button held — resetting preferences and restarting...");

            unsigned long startAttempt = millis();
            ledStatus.setResetting();
            while(millis()-startAttempt < 1000){
              ledStatus.update();
            }
            ledStatus.off();
            ledStatus.update();

            Preferences prefs;
            prefs.begin("wifiConfig", false); // replace with your actual namespace
            prefs.clear();
            prefs.end();

            delay(100);
            ESP.restart();
        }

        vTaskDelay(50 / portTICK_PERIOD_MS);  // Polling delay
    }
}

void startResetButtonTask(gpio_num_t pin, uint32_t holdTimeMs) {
    resetPin = pin;
    holdTime = holdTimeMs;

    xTaskCreatePinnedToCore(
        resetButtonTask,      // Task function
        "ResetButtonTask",    // Task name
        2048,                 // Stack size
        NULL,                 // Params
        1,                    // Priority
        &resetTaskHandle,     // Task handle
        1                     // Core
    );
}
