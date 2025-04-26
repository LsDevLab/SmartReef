#ifndef ACTUATORS_H
#define ACTUATORS_H

#define RELAY_PUMP1        3
#define RELAY_PUMP2        4
#define RELAY_LIGHT        5
#define RELAY_SKIMMER      6
#define RELAY_FILL_PUMP    7
#define RELAY_SPARE_NC     8

extern bool refillPumpActive;
extern bool skimmerActive;
extern bool wavePump1Active;
extern bool wavePump2Active;
extern bool lightActive;

void setupActuators();
void controlActuators();
void refillTankSubcontrol();
void setControlVariables();

#endif
