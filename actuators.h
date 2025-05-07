#ifndef ACTUATORS_H
#define ACTUATORS_H

#include <Arduino.h>

extern bool refillPumpActive;
extern bool wavePump1Active;
extern bool wavePump2Active;
extern bool lightActive;

extern int lightOnHour;
extern int lightOffHour;

void setupActuators();
void controlActuators();
void setControlVariables();
void refillTankSubcontrol();

// control functions
void setLightValue(bool on);
bool getLightValue();
void setWavepump1Value(bool on);
bool getWavepump1Value();
void setWavepump2Value(bool on);
bool getWavepump2Value();

#endif // ACTUATORS_H
