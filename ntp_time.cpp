#include "ntp_time.h"
#include <time.h>
#include <WiFi.h>
#include "configuration.h"

// NTP sync configuration
static const char* ntpServer;
static const char* tzEnv;
static unsigned long lastNtpSync = 0;

void initTime(const char* tz, const char* server) {
  tzEnv = tz;
  ntpServer = server;

  setenv("TZ", tzEnv, 1);  // Set timezone
  tzset();                 // Apply timezone

  Serial.println("Initialized time configuration with NTP.");
}

void syncTimeIfNeeded() {
  if (WiFi.status() != WL_CONNECTED)
    return;

  unsigned long now = millis();
  if (now - lastNtpSync >= NTP_SYNC_INTERVAL || lastNtpSync == 0) {
    Serial.println("Syncing time from NTP...");
    configTzTime(tzEnv, ntpServer);
    delay(5000); // Give time for sync

    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      Serial.print("NTP time: ");
      Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
    } else {
      Serial.println("Failed to obtain time from NTP");
    }

    lastNtpSync = millis();
  }
}

void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain local time");
    return;
  }

  Serial.print("Current time: ");
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}
