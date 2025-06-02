#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <FirebaseClient.h>
#include "sensors.h"
#include "actuators.h"
#include "configuration.h"
#include "webserial_logging.h"
#include <Preferences.h>

extern FirebaseApp app;

void initFirebase();
void uploadStatusToFirestore();
void uploadRefillWaterStatusToFirestore();
void processData(AsyncResult &result);
void updateStatusToFirebaseRTDB(const String &lastUpdated);
void updateRefillStatusToFirebaseRTDB(const String &lastUpdated);
