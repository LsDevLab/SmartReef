#include "ntp_time.h"
#include <time.h>
#include <WiFi.h>
#include "configuration.h"
#include "webserial_logging.h"

// NTP sync configuration
static const char* ntpServer;
static const char* tzEnv;
static unsigned long lastNtpSync = 0;

void initTime(const char* tz, const char* server) {
  tzEnv = tz;
  ntpServer = server;

  setenv("TZ", tzEnv, 1);  // Set timezone
  tzset();                 // Apply timezone

  logPrintln("Initialized time configuration with NTP.");
}

void syncTimeIfNeeded() {
  if (WiFi.status() != WL_CONNECTED)
    return;

  unsigned long now = millis();
  if (now - lastNtpSync >= NTP_SYNC_INTERVAL || lastNtpSync == 0) {
    logPrintln("Syncing time from NTP...");
    configTzTime(tzEnv, ntpServer);
    delay(5000); // Give time for sync

    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      char timeBuffer[64];  // Adjust the size if needed
    
    // Format the time using strftime
    strftime(timeBuffer, sizeof(timeBuffer), "%A, %B %d %Y %H:%M:%S", &timeinfo);
    
    // Log the formatted time string
    logPrint("NTP time: ");
    logPrintln(timeBuffer);  // Using logPrintln to print with a newline
    } else {
      logPrintln("Failed to obtain time from NTP");
    }

    lastNtpSync = millis();
  }
}

void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    logPrintln("Failed to obtain local time");
    return;
  }

  logPrint("Current time: ");
      char timeBuffer[64];  // Adjust the size if needed
    
    // Format the time using strftime
    strftime(timeBuffer, sizeof(timeBuffer), "%A, %B %d %Y %H:%M:%S", &timeinfo);
    
    // Log the formatted time string
    logPrintln(timeBuffer);  // Using logPrintln to print with a newline
}
