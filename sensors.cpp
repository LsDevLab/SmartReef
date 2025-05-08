#include "sensors.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include "configuration.h"
#include "webserial_logging.h"

OneWire oneWire(WATER_TEMP_SENSOR_PIN);
DallasTemperature temperatureSensor(&oneWire);

float tempC = -127;
bool tankFilled = false;

void setupSensors() {
  pinMode(WATER_LEVEL_SENSOR_PIN, INPUT_PULLUP);  // Set digital water level sensor pin
  temperatureSensor.begin();
}

void readAllSensors() {
  readTemp();
  readTankFilledSensor();
}

void readTemp() {
  temperatureSensor.requestTemperatures();
  tempC = temperatureSensor.getTempCByIndex(0);
}

void readTankFilledSensor() {
  tankFilled = digitalRead(WATER_LEVEL_SENSOR_PIN) == HIGH;
}
