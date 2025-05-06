#ifndef ACTUATORS_H
#define ACTUATORS_H

#include <Arduino.h>

// --- Relay Pins ---
#define RELAY_PUMP1        25
#define RELAY_PUMP2        26
#define RELAY_LIGHT        27
#define RELAY_FILL_PUMP    33

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

// Tuya credentials & device IDs
extern const char* TUYA_CLIENT_ID;
extern const char* TUYA_CLIENT_SECRET;
extern const char* TUYA_DEVICE_ID_1;
extern const char* TUYA_DEVICE_ID_2;

// Tapo credentials & device ID
extern const char* TAPO_USERNAME;
extern const char* TAPO_PASSWORD;
extern const char* TAPO_DEVICE_ID;

// control functions
void tuyaSetSwitch(const char* deviceId, bool on);
void tapoControl(const char* deviceIP, bool on);

#endif // ACTUATORS_H
