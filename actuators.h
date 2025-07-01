#ifndef ACTUATORS_H
#define ACTUATORS_H

#include <Arduino.h>

extern bool refillPumpActive;
extern bool fanActive;
extern bool wavePump2Active;
extern bool lightActive;
extern bool forceModeActive;
extern float targetFanTemp;

extern int lightOnHour;
extern int lightOffHour;

void setupActuators();
void controlActuators();
void setControlVariables();
void refillTankSubcontrol();

// control functions
void setLightValue(bool on);
bool getLightValue();
void setRefillPumpValue(bool on);
bool getRefillPumpValue();
//void setWavepump2Value(bool on);
//bool getWavepump2Value();

#endif // ACTUATORS_H
