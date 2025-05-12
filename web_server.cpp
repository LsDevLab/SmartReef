#pragma once

#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>
#include "webserial_logging.h"
#include "configuration.h"
#include <FS.h>
#include <SPIFFS.h>
#include <Preferences.h>

AsyncWebServer server(8083);

void handleCORSPreflight(AsyncWebServerRequest *request) {
  AsyncWebServerResponse *response = request->beginResponse(200);
  response->addHeader("Access-Control-Allow-Origin", "*");
  response->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
  response->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
  request->send(response);
}

void sendCORSHeaders(AsyncWebServerRequest *request, AsyncWebServerResponse *response) {
  response->addHeader("Access-Control-Allow-Origin", "*");
  response->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  response->addHeader("Access-Control-Allow-Headers", "Content-Type");
}

// Route to serve the logs
void setupRoutes() {
  // GET /api/logs
    server.on("/api/logs", HTTP_OPTIONS, [](AsyncWebServerRequest *request) {
    handleCORSPreflight(request);
  });
  server.on("/api/logs", HTTP_GET, [](AsyncWebServerRequest *request) {
    
    File logFile = SPIFFS.open(LOG_FILE_PATH, FILE_READ);
    if (!logFile) {
      AsyncWebServerResponse *response = request->beginResponse(500, "text/plain", "Failed to open log file.");
      sendCORSHeaders(request, response);
      request->send(response);
      return;
    }

    String logContent;
    while (logFile.available()) {
      logContent += (char)logFile.read();
    }
    logFile.close();
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", logContent);
    sendCORSHeaders(request, response);
    request->send(response);
  });

  // POST /api/restart
  server.on("/api/restart", HTTP_OPTIONS, [](AsyncWebServerRequest *request) {
    handleCORSPreflight(request);
  });
  server.on("/api/restart", HTTP_POST, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{\"message\":\"Restarting\"}");
    sendCORSHeaders(request, response);
    request->send(response);
    delay(100);
    ESP.restart();
  });

  // POST /api/reset
  server.on("/api/reset", HTTP_OPTIONS, [](AsyncWebServerRequest *request) {
    handleCORSPreflight(request);
  });
  server.on("/api/reset", HTTP_POST, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{\"message\":\"Restarting\"}");
    sendCORSHeaders(request, response);
    Preferences prefs;
    prefs.begin("wifiConfig", false); // Replace with your actual namespace
    prefs.clear();
    prefs.end();
    request->send(response);
    delay(100);
    ESP.restart();
  });
}

void startWebServer() {
  ElegantOTA.begin(&server);
  ElegantOTA.setAuth(ELEGANT_OTA_USERNAME, ELEGANT_OTA_PASSWORD);

  setupRoutes();

  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
  });

  server.begin();
  logPrintln("Async web server started (OTA + logs).");
}
