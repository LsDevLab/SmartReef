#pragma once

#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>
#include "webserial_logging.h"
#include "configuration.h"
#include <FS.h>
#include <SPIFFS.h>


// Route to serve the logs
void setupRoutes();

void startWebServer();
