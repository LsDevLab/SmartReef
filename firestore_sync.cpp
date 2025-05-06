#include "firestore_sync.h"
#include <WiFi.h>
#include <FirebaseClient.h>
#include "sensors.h"
#include "actuators.h"
#include <WiFiClientSecure.h>
#include "configuration.h"

WiFiClientSecure ssl_client;
using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client);

UserAuth user_auth(API_KEY, USER_EMAIL, USER_PASSWORD, 3000);
FirebaseApp app;
Firestore::Documents Docs;
AsyncResult firestoreResult;

void initFirebase() {
  if (WiFi.status() != WL_CONNECTED) return;
  Serial.println("Initializing Firebase...");
  ssl_client.setInsecure();
  ssl_client.setConnectionTimeout(1000);
  ssl_client.setHandshakeTimeout(5);
  Firebase.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);
  initializeApp(aClient, app, getAuth(user_auth), processData, "🔐 authTask");
  app.getApp<Firestore::Documents>(Docs);
}

String getTimestampString() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return String("Error");
  }
  char buf[80];
  strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &timeinfo);
  return String(buf);
}

unsigned long getTimestampNumeric() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return 0;
  }
  return mktime(&timeinfo);
}

void uploadStatusToFirestore() {
  if (!app.ready() || WiFi.status() != WL_CONNECTED) return;

  unsigned long timestamp = getTimestampNumeric();
  String timestampStr = String(timestamp);

  Values::IntegerValue timestampVal(timestamp);
  Values::BooleanValue waterDetectedVal(digitalRead(WATER_LEVEL_SENSOR_PIN) == LOW);  // LOW means water present
  Values::DoubleValue tempVal(number_t(tempC, 2));
  
  Values::BooleanValue wavePump1Val(wavePump1Active);
  Values::BooleanValue wavePump2Val(wavePump2Active);
  Values::BooleanValue lightVal(lightActive);
  Values::BooleanValue refillPumpVal(refillPumpActive);

  Document<Values::Value> doc;
  doc.add("timestamp", Values::Value(timestampVal));
  doc.add("waterDetected", Values::Value(waterDetectedVal));
  doc.add("tempC", Values::Value(tempVal));
  doc.add("wavePump1", Values::Value(wavePump1Val));
  doc.add("wavePump2", Values::Value(wavePump2Val));
  doc.add("light", Values::Value(lightVal));
  doc.add("refillPump", Values::Value(refillPumpVal));

  String fullDocPath = "status/" + timestampStr;
  Serial.print("Saving status on Firebase...");
  Docs.createDocument(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), fullDocPath, DocumentMask(), doc, processData, "createStatusDoc");

  if (aClient.lastError().code() == 0)
    Serial.println("Done.");
  else {
    Serial.print("Error: ");
    Serial.println(aClient.lastError().message().c_str());
  }
}

void uploadRefillWaterStatusToFirestore() {
  if (!app.ready() || WiFi.status() != WL_CONNECTED) return;

  unsigned long timestamp = getTimestampNumeric();
  String timestampStr = String(timestamp);

  Values::IntegerValue timestampVal(timestamp);
  Values::BooleanValue waterDetectedVal(digitalRead(WATER_LEVEL_SENSOR_PIN) == LOW);
  Values::BooleanValue refillPumpVal(refillPumpActive);

  Document<Values::Value> doc;
  doc.add("timestamp", Values::Value(timestampVal));
  doc.add("waterDetected", Values::Value(waterDetectedVal));
  doc.add("refillPump", Values::Value(refillPumpVal));

  String fullDocPath = "status/" + timestampStr;
  Serial.print("Saving refill status on Firebase...");
  Docs.createDocument(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), fullDocPath, DocumentMask(), doc, processData, "createRefillDoc");

  if (aClient.lastError().code() == 0)
    Serial.println("Done.");
  else {
    Serial.print("Error: ");
    Serial.println(aClient.lastError().message().c_str());
  }
}

void firestoreUploadCallback(AsyncResult &result) {
  if (result.isError()) {
    Serial.printf("Firestore upload failed: %s (code %d)\n", result.error().message().c_str(), result.error().code());
  } else if (result.available()) {
    Serial.println("Firestore document uploaded:");
    Serial.println(result.c_str());
  }
}

void processData(AsyncResult &aResult) {
  if (!aResult.isResult()) return;

  if (aResult.isEvent())
    Firebase.printf("Event: %s | %s (code %d)\n", aResult.uid().c_str(), aResult.eventLog().message().c_str(), aResult.eventLog().code());
  if (aResult.isDebug())
    Firebase.printf("Debug: %s | %s\n", aResult.uid().c_str(), aResult.debug().c_str());
  if (aResult.isError())
    Firebase.printf("Error: %s | %s (code %d)\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());
  if (aResult.available())
    Firebase.printf("Payload: %s | %s\n", aResult.uid().c_str(), aResult.c_str());
}
