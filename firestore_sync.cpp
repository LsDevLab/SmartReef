#include "firestore_sync.h"

#include <FS.h>
#include <SPIFFS.h>

// --- Main SSL client for Firestore & writes ---
static WiFiClientSecure ssl_client;
using AsyncClient = AsyncClientClass;
static AsyncClient aClient(ssl_client);

// --- RTDB stream-specific SSL client ---
static WiFiClientSecure stream_ssl_client;
static AsyncClient streamClient(stream_ssl_client);

// Firebase objects
UserAuth user_auth(API_KEY, USER_EMAIL, USER_PASSWORD, 3000);
FirebaseApp app;
RealtimeDatabase Database;
Firestore::Documents Docs;

AsyncResult firestoreResult, streamResult;

void initFirebase() {
  if (WiFi.status() != WL_CONNECTED) return;

  logPrintln("Initializing Firebase...");

  // SSL Setup
  ssl_client.setInsecure();
  ssl_client.setConnectionTimeout(1000);
  ssl_client.setHandshakeTimeout(5);
  stream_ssl_client.setInsecure();
  stream_ssl_client.setConnectionTimeout(1000);
  stream_ssl_client.setHandshakeTimeout(5);

  Firebase.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);
  //Serial.printf("SPIFFS Total: %u, Used: %u\n", SPIFFS.totalBytes(), SPIFFS.usedBytes());
  //Serial.printf("Free heap: %u\n", ESP.getFreeHeap());

  initializeApp(aClient, app, getAuth(user_auth), processData, "authTask");
  app.getApp<Firestore::Documents>(Docs);
  app.getApp<RealtimeDatabase>(Database);
  Database.url(FIREBASE_DATABASE_URL);

  delay(2000);  // wait for auth

  // --- Open streams for actuators and config ---
  Database.get(streamClient, "/actuators", processData, true, "stream_actuators");
  Database.get(streamClient, "/config", processData, true, "stream_config");

  logPrintln("RTDB streams started on /actuators and /config");
}

String getTimestampString() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    logPrintln("Failed to obtain time");
    return "Error";
  }
  char buf[80];
  strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &timeinfo);
  return String(buf);
}

unsigned long getTimestampNumeric() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    logPrintln("Failed to obtain time");
    return 0;
  }
  return mktime(&timeinfo);
}

void uploadStatusToFirestore() {
  if (!app.ready() || WiFi.status() != WL_CONNECTED) return;

  auto tsNum = getTimestampNumeric();
  String tsStr = String(tsNum);
  updateStatusToFirebaseRTDB(tsStr);

  Document<Values::Value> doc;
  doc.add("timestamp", Values::Value(Values::IntegerValue(tsNum)));
  doc.add("tankFilled", Values::Value(Values::BooleanValue(tankFilled)));
  doc.add("tempC", Values::Value(Values::DoubleValue(number_t(tempC, 2))));
  doc.add("wavePump1", Values::Value(Values::BooleanValue(wavePump1Active)));
  doc.add("wavePump2", Values::Value(Values::BooleanValue(wavePump2Active)));
  doc.add("light", Values::Value(Values::BooleanValue(lightActive)));
  doc.add("refillPump", Values::Value(Values::BooleanValue(refillPumpActive)));

  String fullPath = "status/" + tsStr;
  logPrint("Saving status to Firestore...");
  Docs.createDocument(aClient,
                      Firestore::Parent(FIREBASE_PROJECT_ID),
                      fullPath,
                      DocumentMask(),
                      doc,
                      processData,
                      "createStatusDoc");

  if (aClient.lastError().code() == 0) logPrintln("Done.");
  else {
    logPrint("Error: ");
    logPrintln(aClient.lastError().message().c_str());
  }
}

void uploadRefillWaterStatusToFirestore() {
  if (!app.ready() || WiFi.status() != WL_CONNECTED) return;

  auto tsNum = getTimestampNumeric();
  String tsStr = String(tsNum);
  updateRefillStatusToFirebaseRTDB(tsStr);

  Document<Values::Value> doc;
  doc.add("timestamp", Values::Value(Values::IntegerValue(tsNum)));
  doc.add("tankFilled", Values::Value(Values::BooleanValue(tankFilled)));
  doc.add("refillPump", Values::Value(Values::BooleanValue(refillPumpActive)));

  String fullPath = "status/" + tsStr;
  logPrint("Saving refill status to Firestore...");
  Docs.createDocument(aClient,
                      Firestore::Parent(FIREBASE_PROJECT_ID),
                      fullPath,
                      DocumentMask(),
                      doc,
                      processData,
                      "createRefillDoc");

  if (aClient.lastError().code() == 0) logPrintln("Done.");
  else {
    logPrint("Error: ");
    logPrintln(aClient.lastError().message().c_str());
  }
}

void processData(AsyncResult &result) {
  if (!result.isResult()) return;

  if (result.available()) {
    if (result.to<RealtimeDatabaseResult>().isStream()) {
      auto &r = result.to<RealtimeDatabaseResult>();
      String path = r.dataPath();
      String value = r.to<const char*>();

      logPrintf("[STREAM] Event: %s | Path: %s | Value: %s\n",
                r.event().c_str(), path.c_str(), value.c_str());

      // --- Actuator Updates ---
      if (path == "/refillPumpActive") {
        refillPumpActive = value == "true";
        digitalWrite(RELAY_FILL_PUMP, refillPumpActive ? LOW : HIGH);
        logPrintln("Updated: refillPumpActive");

      } else if (path == "/wavePump1Active") {
        wavePump1Active = value == "true";
        setWavepump1Value(wavePump1Active);
        logPrintln("Updated: wavePump1Active");

      } else if (path == "/wavePump2Active") {
        wavePump2Active = value == "true";
        setWavepump2Value(wavePump2Active);
        logPrintln("Updated: wavePump2Active");

      } else if (path == "/lightActive") {
        lightActive = value == "true";
        setLightValue(lightActive);
        logPrintln("Updated: lightActive");

      // --- Config Updates ---
      } else if (path == "/lightOnHour") {
        lightOnHour = atoi(value.c_str());
        logPrintf("Updated: lightOnHour = %d\n", lightOnHour);

      } else if (path == "/lightOffHour") {
        lightOffHour = atoi(value.c_str());
        logPrintf("Updated: lightOffHour = %d\n", lightOffHour);
      }

    } else {
      // Non-stream callback result (e.g. Firestore write)
      Serial.printf("[CALLBACK] Task: %s | Data: %s\n",
                    result.uid().c_str(), result.c_str());
    }
  }

  if (result.isError()) {
    logPrintf("[ERROR] %s: %s (code %d)\n",
              result.uid().c_str(),
              result.error().message().c_str(),
              result.error().code());
  }
}

void updateStatusToFirebaseRTDB(const String &lastUpdated) {
  Database.set<number_t>(aClient, "/sensors/temperatureC", number_t(tempC, 2), processData, "setTemp");
  Database.set<bool>(aClient, "/sensors/tankFilled", tankFilled, processData, "setTank");
  Database.set<bool>(aClient, "/actuators/refillPumpActive", refillPumpActive, processData, "setRefillPump");
  Database.set<bool>(aClient, "/actuators/wavePump1Active", wavePump1Active, processData, "setWave1");
  Database.set<bool>(aClient, "/actuators/wavePump2Active", wavePump2Active, processData, "setWave2");
  Database.set<bool>(aClient, "/actuators/lightActive", lightActive, processData, "setLight");
  Database.set<int>(aClient, "/config/lightOnHour", lightOnHour, processData, "setOnHour");
  Database.set<int>(aClient, "/config/lightOffHour", lightOffHour, processData, "setOffHour");
  Database.set<String>(aClient, "/lastUpdated", lastUpdated, processData, "setLastUpdated");
}

void updateRefillStatusToFirebaseRTDB(const String &lastUpdated) {
  Database.set<bool>(aClient, "/sensors/tankFilled", tankFilled, processData, "setTankRefill");
  Database.set<bool>(aClient, "/actuators/refillPumpActive", refillPumpActive, processData, "setRefillPumpRefill");
  Database.set<String>(aClient, "/lastUpdated", lastUpdated, processData, "setLastUpdatedRefill");
}
