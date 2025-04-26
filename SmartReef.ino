#include "sensors.h"
#include "actuators.h"
#include "status.h"
#include "wifi_setup.h"
#include "led_status.h"
#include "ntp_time.h"
#include "firestore_sync.h"
#include "reset_task.h"
#include <ElegantOTA.h>

#define CYCLE_TIME    60000     

void setup() {
  Serial.begin(115200);
  delay(1000); // Give time to initialize serial

  Serial.println("\nStarting Smart Aquarium System (v.0.0.1)...\n");

  startResetButtonTask((gpio_num_t)17, 5000);
  
  setupNetwork(); 

  initTime("CET-1CEST,M3.5.0/2,M10.5.0/3", "pool.ntp.org");
  syncTimeIfNeeded();

  initFirebase();
  
 //setupSensors();
 
  setupActuators();
  
}

void loop() {

    checkNetwork();

    syncTimeIfNeeded();

    //readAllSensors();

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

}

//not use com usb port
