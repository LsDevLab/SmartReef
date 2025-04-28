#include <Arduino.h>
#include <esp_sleep.h>
#include "sensors.h"
#include "actuators.h"
#include "ntp_time.h"

void printStatus() {
  Serial.println("\n===================== Aquarium Status =====================");

  printLocalTime();

  // Sensor Readings
  Serial.println("------------------------  Sensors -------------------------");

  if (waterLevelCm >= 0) {
    Serial.print(" ❌         Water Level (cm):  ");
    Serial.println(waterLevelCm);
  } else {
    Serial.print(" ❌         Water Level (cm):  Error - Out of range\n");
  }

  if (tempC != DEVICE_DISCONNECTED_C) {
    Serial.print("      Water Temperature (°C):  ");
    Serial.println(tempC);
  } else {
    Serial.print("      Water Temperature (°C):  Error - Sensor disconnected\n");
  }

    Serial.print("         Ambient Light (lux):  ");
  Serial.println(lux);

  // ⚙️ Actuator States
  Serial.println("------------------------- Actuators ----------------------- ");
    Serial.print("                 Wave Pump 1:  "); Serial.println(wavePump1Active ? "ON" : "OFF");
    Serial.print("                 Wave Pump 2:  "); Serial.println(wavePump2Active ? "ON" : "OFF");
    Serial.print("                     Skimmer:  "); Serial.println(skimmerActive ? "ON" : "OFF");
    Serial.print("                       Light:  "); Serial.println(lightActive ? "ON" : "OFF");
    Serial.print("                 Refill Pump:  "); Serial.println(refillPumpActive ? "ON" : "OFF");

}
