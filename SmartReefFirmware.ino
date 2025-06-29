//#define CORE_DEBUG_LEVEL 4


#include "sensors.h"
#include "actuators.h"
#include "status.h"
#include "wifi_setup.h"
#include "led_status.h"
#include "ntp_time.h"
#include "firestore_sync.h"
#include "reset_task.h"
#include <ElegantOTA.h>
#include "configuration.h"
#include "webserial_logging.h"
#include <FS.h>
#include <SPIFFS.h>

Preferences prefs;

void setup() {
  Serial.begin(115200);
  //Serial.setDebugOutput(true);  // Required for internal log_x macros to show output
  delay(1000); // Give time to initialize serial

  if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed");
    }

Serial.printf("Total: %u bytes, Used: %u bytes\n", SPIFFS.totalBytes(), SPIFFS.usedBytes());


  logPrintln("\nStarting Smart Aquarium System (v.0.0.1)...\n");
  
  ledStatus.begin();

  setupResetButton();
  
  setupNetwork(); 

  initTime("CET-1CEST,M3.5.0/2,M10.5.0/3", "pool.ntp.org");
  syncTimeIfNeeded();

  initFirebase();
  
  setupSensors();
 
  setupActuators();
  
}

void loop() {

    readAllSensors();

    printStatus();

    uploadStatusToFirestore();

    controlActuators();

    unsigned long startAttempt = millis();
    ledStatus.off();
    while(millis()-startAttempt < CYCLE_TIME){
      ledStatus.update();
      app.loop();
      ElegantOTA.loop();
    }

    checkNetwork();

    syncTimeIfNeeded();

}
