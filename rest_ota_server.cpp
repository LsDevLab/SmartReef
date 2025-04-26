#include "rest_ota_server.h"
#include <ElegantOTA.h>
#include "sensors.h"
#include "actuators.h"
#include <ArduinoJson.h>
#include <Preferences.h>

WebServer server(80);

// Forward declaration of the task
void webServerTask(void *parameter);


// Helper function to handle actuator update
void handleActuatorUpdate(bool &actuatorVar, uint8_t relayPin) {
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"error\":\"Missing body\"}");
    return;
  }

  DynamicJsonDocument doc(128);
  DeserializationError error = deserializeJson(doc, server.arg("plain"));

  if (error || !doc.containsKey("value")) {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON or missing 'value'\"}");
    return;
  }

  actuatorVar = doc["value"];
  digitalWrite(relayPin, actuatorVar ? HIGH : LOW);

  server.send(200, "application/json", "{\"message\":\"Actuator updated\"}");
}

void setupRoutes() {
  

  server.on("/api/status", HTTP_GET, []() {
    // Dynamically return uptime in seconds
    unsigned long uptime = millis() / 1000;
    String response = "{\"status\":\"ok\",\"uptime\":" + String(uptime) + "}";
    server.send(200, "application/json", response);
  });


 // GET /api/actuators
  server.on("/api/actuators", HTTP_GET, []() {
    DynamicJsonDocument doc(256);
    doc["refillPumpActive"] = refillPumpActive;
    doc["skimmerActive"] = skimmerActive;
    doc["wavePump1Active"] = wavePump1Active;
    doc["wavePump2Active"] = wavePump2Active;
    doc["lightActive"] = lightActive;
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
  });

  // GET /api/sensors
  server.on("/api/sensors", HTTP_GET, []() {
    DynamicJsonDocument doc(256);
    doc["waterLevelCm"] = waterLevelCm;
    doc["temperatureC"] = tempC;
    doc["lux"] = lux;
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
  });

  // POST /api/actuators/refillPumpActive
  server.on("/api/actuators/refillPump", HTTP_POST, []() {
    handleActuatorUpdate(refillPumpActive, RELAY_FILL_PUMP);
  });
  
  // POST /api/actuators/skimmerActive
  server.on("/api/actuators/skimmer", HTTP_POST, []() {
    handleActuatorUpdate(skimmerActive, RELAY_SKIMMER);
  });
  
  // POST /api/actuators/wavePump1Active
  server.on("/api/actuators/wavePump1", HTTP_POST, []() {
    handleActuatorUpdate(wavePump1Active, RELAY_PUMP1);
  });
  
  // POST /api/actuators/wavePump2Active
  server.on("/api/actuators/wavePump2", HTTP_POST, []() {
    handleActuatorUpdate(wavePump2Active, RELAY_PUMP2);
  });
  
  // POST /api/actuators/lightActive
  server.on("/api/actuators/light", HTTP_POST, []() {
    handleActuatorUpdate(lightActive, RELAY_LIGHT);
  });

  // POST /api/restart
  server.on("/api/restart", HTTP_POST, []() {
    server.send(200, "application/json", "{\"message\":\"Restarting\"}");
    delay(100);
    ESP.restart();
  });

  // POST /api/resetprefs
  server.on("/api/reset", HTTP_POST, []() {
    Preferences prefs;
    prefs.begin("wifiConfig", false); // Replace "wificonfig" with your namespace
    prefs.clear();
    prefs.end();
    server.send(200, "application/json", "{\"message\":\"Preferences cleared\"}");
    delay(100);
    ESP.restart();
  });

  // ElegantOTA
  ElegantOTA.begin(&server);
  ElegantOTA.setAuth("admin", "superpaguro");

  server.onNotFound([]() {
    server.send(404, "text/plain", "Not found");
  });
}

void startRestOtaServer() {
  setupRoutes();
  server.begin();
  Serial.println("REST+ElegantOTA webserver initialized.");

  // Create FreeRTOS task for handling HTTP
  xTaskCreatePinnedToCore(
    webServerTask,      // Task function
    "RESTOTAServerTask",     // Name
    4096,                // Stack size
    NULL,                // Parameters
    1,                   // Priority
    NULL,                // Task handle
    0                    // Core (0 is usually good for WiFi+HTTP)
  );
}

void webServerTask(void *parameter) {
  for (;;) {
    server.handleClient();
    vTaskDelay(1); // small delay to allow other tasks (important!)
  }
}
