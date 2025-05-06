#ifndef REST_OTA_SERVER_H
#define REST_OTA_SERVER_H

#include <WiFi.h>
#include <WebServer.h>

extern WebServer server;

void startRestOtaServer();
void handleConfigUpdate();

#endif // REST_OTA_SERVER_H
