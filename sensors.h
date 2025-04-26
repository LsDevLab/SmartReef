#ifndef SENSORS_H
#define SENSORS_H

#include "Adafruit_VL53L0X.h"
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 2
#define LIGHTSENSORPIN A1

extern Adafruit_VL53L0X waterLevelSensor;
extern OneWire oneWire;
extern DallasTemperature temperatureSensor;
extern float waterLevelCm, tempC, lux;

void setupSensors();

void readWaterLevel();
void readTemp();
void readLux();

void readAllSensors();

#endif
