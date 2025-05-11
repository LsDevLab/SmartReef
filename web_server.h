#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <WiFi.h>
#include <ESPAsyncWebServer.h>

extern AsyncWebServer server;

void startWebServer();

#endif // WEB_SERVER_H
