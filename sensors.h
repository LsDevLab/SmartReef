#pragma once
#include <DallasTemperature.h> 

extern float tempC;
extern bool tankFilled;

void setupSensors();
void readAllSensors();
void readTemp();
void readTankFilledSensor();
