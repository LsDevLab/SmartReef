#include <Arduino.h>
#include <esp_sleep.h>
#include "sensors.h"
#include "actuators.h"
#include "ntp_time.h"
#include "configuration.h"

void printStatus() {
  Serial.println("\n===================== Aquarium Status =====================");

  printLocalTime();

  // Sensor Readings
  Serial.println("------------------------  Sensors -------------------------");

  // Digital water level sensor: LOW = Water present, HIGH = No water
  Serial.print("      Water Level Sensor:  ");
  Serial.println(tankFilled ? "WATER DETECTED" : "LOW / NO WATER");

  if (tempC != DEVICE_DISCONNECTED_C) {
    Serial.print("      Water Temperature (°C):  ");
    Serial.println(tempC);
  } else {
    Serial.print("      Water Temperature (°C):  Error - Sensor disconnected\n");
  }

  // Actuator States
  Serial.println("------------------------- Actuators -----------------------");
  Serial.print("                 Wave Pump 1:  "); Serial.println(wavePump1Active ? "ON" : "OFF");
  Serial.print("                 Wave Pump 2:  "); Serial.println(wavePump2Active ? "ON" : "OFF");
  Serial.print("                       Light:  "); Serial.println(lightActive ? "ON" : "OFF");
  Serial.print("                 Refill Pump:  "); Serial.println(refillPumpActive ? "ON" : "OFF");
}
