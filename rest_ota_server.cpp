#include "rest_ota_server.h"
#include <ElegantOTA.h>
#include "sensors.h"
#include "actuators.h"
#include <ArduinoJson.h>
#include <Preferences.h>
#include "configuration.h"

WebServer server(80);

// Forward declaration of the task
void webServerTask(void *parameter);

int getDocValue(){
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"error\":\"Missing body\"}");
    return -1;
  }

  DynamicJsonDocument doc(128);
  DeserializationError error = deserializeJson(doc, server.arg("plain"));

  if (error || !doc.containsKey("value")) {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON or missing 'value'\"}");
    return -1;
  }

  return doc["value"];
}


// Helper function to handle actuator update
void handleActuatorUpdate(bool &actuatorVar, uint8_t relayPin) {
  int docValue = getDocValue();
  if(docValue == -1) return;

  actuatorVar = docValue;
  digitalWrite(relayPin, actuatorVar ? LOW : HIGH);

  server.send(200, "application/json", "{\"message\":\"Actuator updated\"}");
}

void handleTapoLightUpdate() {
  int docValue = getDocValue();
  if(docValue == -1) return;

  lightActive = docValue;
  setLightValue(lightActive);

  server.send(200, "application/json", "{\"message\":\"Actuator updated\"}");
}

void handleTuyaWavePump1Update() {
  int docValue = getDocValue();
  if(docValue == -1) return;

  wavePump1Active = docValue;
  setWavepump1Value(wavePump1Active);

  server.send(200, "application/json", "{\"message\":\"Actuator updated\"}");
}

void handleTuyaWavePump2Update() {
  int docValue = getDocValue();
  if(docValue == -1) return;

  wavePump2Active = docValue;
  setWavepump1Value(wavePump2Active);

  server.send(200, "application/json", "{\"message\":\"Actuator updated\"}");
}

void handleConfigUpdate(int &configVar) {
  int docValue = getDocValue();
  if(docValue == -1) return;
  
  configVar = docValue;

  server.send(200, "application/json", "{\"message\":\"Config updated\"}");
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
    lightActive = getLightValue();
    wavePump1Active = getWavepump1Value();
    wavePump2Active = getWavepump2Value();
    DynamicJsonDocument doc(256);
    doc["refillPumpActive"] = refillPumpActive;
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
    doc["tankFilled"] = tankFilled;
    doc["temperatureC"] = tempC;
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
  });
  
    // GET /api/config
  server.on("/api/config", HTTP_GET, []() {
    DynamicJsonDocument doc(256);
    doc["lightOnHour"] = lightOnHour;
    doc["lightOffHour"] = lightOffHour;
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
  });

  // POST /api/actuators/refillPumpActive
  server.on("/api/actuators/refillPump", HTTP_POST, []() {
    handleActuatorUpdate(refillPumpActive, RELAY_FILL_PUMP);
  });
  
  // POST /api/actuators/wavePump1Active
  server.on("/api/actuators/wavePump1", HTTP_POST, []() {
    handleTuyaWavePump1Update();
  });
  
  // POST /api/actuators/wavePump2Active
  server.on("/api/actuators/wavePump2", HTTP_POST, []() {
    handleTuyaWavePump2Update();
  });
  
  // POST /api/actuators/lightActive
  server.on("/api/actuators/light", HTTP_POST, []() {
    handleTapoLightUpdate();
  });

  
  // POST /api/config/lightOnHour
  server.on("/api/config/lightOnHour", HTTP_POST, []() {
    handleConfigUpdate(lightOnHour);
  });
    
  // POST /api/config/lightOffHour
  server.on("/api/config/lightOffHour", HTTP_POST, []() {
     handleConfigUpdate(lightOffHour);
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
  ElegantOTA.setAuth(ELEGANT_OTA_USERNAME, ELEGANT_OTA_PASSWORD);

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
