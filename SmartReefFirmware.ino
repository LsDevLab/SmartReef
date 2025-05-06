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
//#include "rc522_rfid.h" 


void setup() {
  Serial.begin(115200);
  delay(1000); // Give time to initialize serial

  Serial.println("\nStarting Smart Aquarium System (v.0.0.1)...\n");
  
  ledStatus.begin();

  setupResetButton();

  //initRFID();
  
  setupNetwork(); 

  initTime("CET-1CEST,M3.5.0/2,M10.5.0/3", "pool.ntp.org");
  syncTimeIfNeeded();

  initFirebase();
  
   setupSensors();
 
  setupActuators();
  
}

void loop() {

    readAllSensors();

    controlActuators();

    printStatus();

    uploadStatusToFirestore();

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

//not use com usb port
