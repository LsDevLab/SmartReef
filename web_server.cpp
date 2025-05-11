#include "web_server.h"
#include <ElegantOTA.h>
#include "sensors.h"
#include "actuators.h"
#include <ArduinoJson.h>
#include <Preferences.h>
#include "configuration.h"
#include "webserial_logging.h"
#include <FS.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include "cert.h"

// Create an instance of the server
AsyncWebServer server(443);  // Using HTTPS on port 443

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
  setWavepump2Value(wavePump2Active);

  server.send(200, "application/json", "{\"message\":\"Actuator updated\"}");
}

void handleConfigUpdate(int &configVar) {
  int docValue = getDocValue();
  if(docValue == -1) return;
  
  configVar = docValue;

  server.send(200, "application/json", "{\"message\":\"Config updated\"}");
}

void setupRoutes() {
  // Status API
  server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request){
    unsigned long uptime = millis() / 1000;
    String response = "{\"status\":\"ok\",\"uptime\":" + String(uptime) + "}";
    request->send(200, "application/json", response);
  });

  // Actuators API
  server.on("/api/actuators", HTTP_GET, [](AsyncWebServerRequest *request){
    lightActive = getLightValue();
    DynamicJsonDocument doc(256);
    doc["refillPumpActive"] = refillPumpActive;
    doc["wavePump1Active"] = wavePump1Active;
    doc["wavePump2Active"] = wavePump2Active;
    doc["lightActive"] = lightActive;
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });

  // Sensors API
  server.on("/api/sensors", HTTP_GET, [](AsyncWebServerRequest *request){
    DynamicJsonDocument doc(256);
    doc["tankFilled"] = tankFilled;
    doc["temperatureC"] = tempC;
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });

  // Config API
  server.on("/api/config", HTTP_GET, [](AsyncWebServerRequest *request){
    DynamicJsonDocument doc(256);
    doc["lightOnHour"] = lightOnHour;
    doc["lightOffHour"] = lightOffHour;
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });

  // Logs API
  server.on("/api/logs", HTTP_GET, [](AsyncWebServerRequest *request){
    File logFile = SPIFFS.open(LOG_FILE_PATH, FILE_READ);
    if (!logFile) {
        request->send(500, "text/plain", "Failed to open log file.");
        return;
    }
    String logContent = "";
    while (logFile.available()) {
        logContent += (char)logFile.read();
    }
    logFile.close();
    request->send(200, "text/plain", logContent);
  });

  // Actuators POST APIs
  server.on("/api/actuators/refillPump", HTTP_POST, [](AsyncWebServerRequest *request){
    handleActuatorUpdate(refillPumpActive, RELAY_FILL_PUMP);
  });

  server.on("/api/actuators/wavePump1", HTTP_POST, [](AsyncWebServerRequest *request){
    handleTuyaWavePump1Update();
  });

  server.on("/api/actuators/wavePump2", HTTP_POST, [](AsyncWebServerRequest *request){
    handleTuyaWavePump2Update();
  });

  server.on("/api/actuators/light", HTTP_POST, [](AsyncWebServerRequest *request){
    handleTapoLightUpdate();
  });

  // Config Update APIs
  server.on("/api/config/lightOnHour", HTTP_POST, [](AsyncWebServerRequest *request){
    handleConfigUpdate(lightOnHour);
  });

  server.on("/api/config/lightOffHour", HTTP_POST, [](AsyncWebServerRequest *request){
    handleConfigUpdate(lightOffHour);
  });

  // Restart API
  server.on("/api/restart", HTTP_POST, [](AsyncWebServerRequest *request){
    request->send(200, "application/json", "{\"message\":\"Restarting\"}");
    delay(100);
    ESP.restart();
  });

  // Reset Preferences API
  server.on("/api/reset", HTTP_POST, [](AsyncWebServerRequest *request){
    Preferences prefs;
    prefs.begin("wifiConfig", false);
    prefs.clear();
    prefs.end();
    request->send(200, "application/json", "{\"message\":\"Preferences cleared\"}");
    delay(100);
    ESP.restart();
  });

  // Elegant OTA
  ElegantOTA.begin(&server);
  ElegantOTA.setAuth(ELEGANT_OTA_USERNAME, ELEGANT_OTA_PASSWORD);

  // Not found handler
  server.onNotFound([](AsyncWebServerRequest *request){
    request->send(404, "text/plain", "Not found");
  });
}

void startWebServer() {
  // Set the certificate and key for HTTPS
  server.getServer().setRSACert(server_cert, server_key);
  
  setupRoutes();  // Setup all the routes
  server.begin();
  logPrintln("Webserver initialized.");
  
  // No need for webServerTask with AsyncWebServer
}
