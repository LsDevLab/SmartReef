#include "firestore_sync.h"
#include <WiFi.h>
#include <FirebaseClient.h>
#include "sensors.h"
#include "actuators.h"
#include <WiFiClientSecure.h>


String getTimestampString();
void processData(AsyncResult &aResult);

WiFiClientSecure ssl_client;

// This uses built-in core WiFi/Ethernet for network connection.
// See examples/App/NetworkInterfaces for more network examples.
using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client);


LegacyToken legacy_token(DATABASE_SECRET);
FirebaseApp app;

Firestore::Documents Docs;

AsyncResult firestoreResult;


void initFirebase() {

  if (WiFi.status() != WL_CONNECTED)
    return;
  
  Serial.println("Initializing Firebase...");

    // Configure SSL client
  ssl_client.setInsecure();
  ssl_client.setConnectionTimeout(1000);
  ssl_client.setHandshakeTimeout(5);
  

  Firebase.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);


    Serial.println("Initializing app...");
  initializeApp(aClient, app, getAuth(legacy_token), processData, "🔐 authTask");

    // Or intialize the app and wait.
    // initializeApp(aClient, app, getAuth(user_auth), 120 * 1000, auth_debug_print);

    app.getApp<Firestore::Documents>(Docs);
}

String getTimestampString()
{
    // Obtain current time from NTP
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return String("Error");
    }

    // Format the time string
    char buf[80];
    String format = "%Y-%m-%dT%H:%M:%S";

    // Get the current system time in nanoseconds (if needed for precision)
    uint32_t nano = 0; // Set nano seconds to 0 or calculate if needed

    if (nano > 0)
    {
        String fraction = String(double(nano) / 1000000000.0f, 9);
        fraction.remove(0, 1); // Remove leading "0."
        format += fraction;
    }
    format += "Z";  // Add the 'Z' indicating UTC time

    // Convert time structure to string with the specified format
    strftime(buf, sizeof(buf), format.c_str(), &timeinfo);

    return buf;
}

unsigned long getTimestampNumeric() {
    struct tm timeinfo;
    
    // Get the current time
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return 0;
    }

    // Convert local time to time_t (seconds since Unix epoch)
    unsigned long timestamp = mktime(&timeinfo);

    // Return numeric timestamp (seconds since Unix epoch)
    return timestamp;
}


void uploadStatusToFirestore() {
  
  if (!app.ready() || WiFi.status() != WL_CONNECTED)
    return;
    
  long unsigned int timestamp = getTimestampNumeric();
  String timestampStr = String(timestamp); // Convert timestamp to string for document ID

  Values::IntegerValue timestampVal(timestamp);
  Values::DoubleValue waterLevelVal(number_t(waterLevelCm, 2));
  Values::DoubleValue tempVal(number_t(tempC, 2));
  Values::DoubleValue luxVal(number_t(lux, 2));
  Values::BooleanValue wavePump1Val(wavePump1Active);
  Values::BooleanValue wavePump2Val(wavePump2Active);
  Values::BooleanValue skimmerVal(skimmerActive);
  Values::BooleanValue lightVal(lightActive);
  Values::BooleanValue refillPumpVal(refillPumpActive);
  
  Document<Values::Value> doc; // Use the timestamp as the document ID

  // Add values to the document
  doc.add("timestamp", Values::Value(timestampVal));
  doc.add("waterLevelCm", Values::Value(waterLevelVal));
  doc.add("tempC", Values::Value(tempVal));
  doc.add("lux", Values::Value(luxVal));
  doc.add("wavePump1", Values::Value(wavePump1Val));
  doc.add("wavePump2", Values::Value(wavePump2Val));
  doc.add("skimmer", Values::Value(skimmerVal));
  doc.add("light", Values::Value(lightVal));
  doc.add("refillPump", Values::Value(refillPumpVal));

  // Full Firestore path (no need for a trailing slash)
  String docPath = "status";

  // Upload document to Firestore
  Serial.print("Saving status on Firebase...");

  // Sync call which waits until the payload is received
  String fullDocPath = "status/" + timestampStr;
  String payload = Docs.createDocument(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), fullDocPath, DocumentMask(), doc); 
  
  if (aClient.lastError().code() == 0)
    Serial.println("Done.");
  else {
    Serial.print("Error:");
    Serial.println(aClient.lastError().message().c_str());
    Firebase.printf("Error, msg: %s, code: %d\n", aClient.lastError().message().c_str(), aClient.lastError().code());
  }
}


void uploadRefillWaterStatusToFirestore() {
  
  if (!app.ready() || WiFi.status() != WL_CONNECTED)
    return;
    
  long unsigned int timestamp = getTimestampNumeric();
  String timestampStr = String(timestamp); // Convert timestamp to string for document ID

  Values::IntegerValue timestampVal(timestamp);
  Values::DoubleValue waterLevelVal(number_t(waterLevelCm, 2));
  Values::BooleanValue refillPumpVal(refillPumpActive);
  
  Document<Values::Value> doc; // Use the timestamp as the document ID

  // Add values to the document
  doc.add("timestamp", Values::Value(timestampVal));
  doc.add("waterLevelCm", Values::Value(waterLevelVal));
  doc.add("refillPump", Values::Value(refillPumpVal));

  // Upload document to Firestore
  Serial.print("Saving water fill status on Firebase...");

  // Sync call which waits until the payload is received
  String fullDocPath = "status/" + timestampStr;

    // Sync call which waits until the payload was received.
    Docs.createDocument(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), fullDocPath, DocumentMask(), doc, processData, "createDocumentTask");
    if (aClient.lastError().code() == 0)
        Serial.println("Done.");
    else {
        Serial.print("Error:");
        Serial.println(aClient.lastError().message().c_str());
        Firebase.printf("Error, msg: %s, code: %d\n", aClient.lastError().message().c_str(), aClient.lastError().code());
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
  if (!aResult.isResult())
    return;

  if (aResult.isEvent())
    Firebase.printf("Event task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.eventLog().message().c_str(), aResult.eventLog().code());

  if (aResult.isDebug())
    Firebase.printf("Debug task: %s, msg: %s\n", aResult.uid().c_str(), aResult.debug().c_str());

  if (aResult.isError())
    Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());

  if (aResult.available())
    Firebase.printf("task: %s, payload: %s\n", aResult.uid().c_str(), aResult.c_str());
}
