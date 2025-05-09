#include <Arduino.h>
#include <esp_sleep.h>
#include "sensors.h"
#include "actuators.h"
#include "ntp_time.h"
#include "configuration.h"
#include "webserial_logging.h"

void printStatus() {
  logPrintln("===================== Aquarium Status =====================");

  printLocalTime();

  // Sensor Readings
  logPrintln("------------------------  Sensors -------------------------");

  // Digital water level sensor: LOW = Water present, HIGH = No water
  logPrint("Water Level Sensor: ");
  logPrintln(tankFilled ? "WATER DETECTED" : "LOW / NO WATER");

  if (tempC != DEVICE_DISCONNECTED_C) {
    logPrint("Water Temperature (°C): ");
    logPrintln(tempC);
  } else {
    logPrint("Water Temperature (°C):  Error - Sensor disconnected\n");
  }

  // Actuator States
  logPrintln("------------------------- Actuators -----------------------");
  logPrint("Wave Pump 1: "); logPrintln(wavePump1Active ? "ON" : "OFF");
  logPrint("Wave Pump 2: "); logPrintln(wavePump2Active ? "ON" : "OFF");
  logPrint("Light: "); logPrintln(lightActive ? "ON" : "OFF");
  logPrint("Refill Pump: "); logPrintln(refillPumpActive ? "ON" : "OFF");
}
