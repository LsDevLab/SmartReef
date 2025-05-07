
#ifndef TUYA_DEVICE_H
#define TUYA_DEVICE_H

#include <Arduino.h>

void tuyaAuthenticate();
void tuyaSetSwitch(const char* deviceId, bool on);
bool tuyaGetSwitch(const char* deviceId);

#endif // TUYA_DEVICE_H
