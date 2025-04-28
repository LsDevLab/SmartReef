#include "ntp_time.h"
#include <time.h>
#include <Wifi.h>

static const char* ntpServer;
static const char* tzEnv;

static unsigned long lastNtpSync = 0;
static const unsigned long syncInterval = 3600UL * 1000UL; // 1 hour in ms


void initTime(const char* tz, const char* server) {
  tzEnv = tz;
  ntpServer = server;
  setenv("TZ", tzEnv, 1);  // Set timezone
  tzset();                 // Apply timezone
}

void syncTimeIfNeeded() {
   if (WiFi.status() != WL_CONNECTED)
    return;
  
  unsigned long now = millis();
  if (now - lastNtpSync >= syncInterval || lastNtpSync == 0) {
    Serial.println("Syncing time from NTP...");
    configTzTime(tzEnv, ntpServer);
    lastNtpSync = millis();
    delay(5000); // Wait for sync
    printLocalTime();
  }
}

void printLocalTime() {
  if (WiFi.status() != WL_CONNECTED)
    return;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.print("Current time: ");
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}
