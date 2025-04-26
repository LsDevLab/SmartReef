#include "actuators.h"
#include "sensors.h"
#include "led_status.h"
#include "firestore_sync.h"

bool refillPumpActive = false;
bool skimmerActive = true;
bool wavePump1Active = true;
bool wavePump2Active = true;
bool lightActive = true;


void setupActuators() {
  pinMode(RELAY_PUMP1, OUTPUT);
  pinMode(RELAY_PUMP2, OUTPUT);
  pinMode(RELAY_LIGHT, OUTPUT);
  pinMode(RELAY_SKIMMER, OUTPUT);
  pinMode(RELAY_FILL_PUMP, OUTPUT);
  pinMode(RELAY_SPARE_NC, OUTPUT);

  digitalWrite(RELAY_PUMP1, wavePump1Active);
  digitalWrite(RELAY_PUMP2, wavePump2Active);
  digitalWrite(RELAY_LIGHT, lightActive);
  digitalWrite(RELAY_SKIMMER, skimmerActive);
  digitalWrite(RELAY_FILL_PUMP, refillPumpActive);
  digitalWrite(RELAY_SPARE_NC, HIGH);

  Serial.println("Testing controls...");
  delay(2000);
}

void controlActuators() {

  setControlVariables();

  digitalWrite(RELAY_PUMP1, wavePump1Active);
  digitalWrite(RELAY_PUMP2, wavePump2Active);
  digitalWrite(RELAY_LIGHT, lightActive);
  digitalWrite(RELAY_SKIMMER, skimmerActive);
  digitalWrite(RELAY_SPARE_NC, HIGH);

  refillTankSubcontrol();
  
}

void setControlVariables() {

  if (WiFi.status() != WL_CONNECTED){
    skimmerActive = true;
    wavePump1Active = true;
    wavePump2Active = true;
    lightActive = true;
    return;
  }
  
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time in setControlVariables");
    wavePump1Active = true;
    wavePump2Active = true;
    lightActive = true;
    return;
  }

  // Alternate wave pumps every 1 hour
  if (timeinfo.tm_hour % 2 == 0) {
    wavePump1Active = true;
    wavePump2Active = false;
  } else {
    wavePump1Active = false;
    wavePump2Active = true;
  }

  // Light ON from 17:00 to 23:59
  if (timeinfo.tm_hour >= 17 && timeinfo.tm_hour < 24) {
    lightActive = true;
  } else {
    lightActive = false;
  }
}

void refillTankSubcontrol() {

  //readWaterLevel();
  
  if (waterLevelCm >= 0) {
    while (waterLevelCm >= 43.75 && waterLevelCm <= 44.0) {
      digitalWrite(RELAY_FILL_PUMP, HIGH);
      ledStatus.setWaterRefilling();
      ledStatus.update();
      refillPumpActive = true;
      Serial.print("Refill Pump: ");
      Serial.println(refillPumpActive ? "ON" : "OFF");
      uploadRefillWaterStatusToFirestore();
      delay(1000);
      //readWaterLevel();
    }
    digitalWrite(RELAY_FILL_PUMP, LOW);
    refillPumpActive = false;
  }
  
}
