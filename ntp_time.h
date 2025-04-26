#pragma once
#include <Arduino.h>

void initTime(const char* tz, const char* ntpServer);
void syncTimeIfNeeded();
void printLocalTime();
