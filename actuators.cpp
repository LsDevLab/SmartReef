#include "actuators.h"
#include "sensors.h"
#include "led_status.h"
#include "firestore_sync.h"
#include <WiFi.h>
#include "configuration.h"
#define TAPO_DEBUG_MODE // Comment this line to disable debug messages
#include "tapo_device.h"
#include "tuya_device.h"


// your existing flags
bool wavePump1Active = true;
bool wavePump2Active = true;
bool lightActive    = true;
bool refillPumpActive = false;

// light schedule
int lightOnHour  = 17;
int lightOffHour = 24;

TapoDevice tapoLight;

void setupActuators() {
  pinMode(RELAY_FILL_PUMP, OUTPUT);
  digitalWrite(RELAY_FILL_PUMP, !refillPumpActive);
  tapoLight.begin(TAPO_LIGHT_IP, TAPO_USERNAME, TAPO_PASSWORD);
  lightActive = getLightValue();
  wavePump1Active = getWavepump1Value();
  wavePump2Active = getWavepump2Value();
  Serial.println("Refill pump control initialized.");
  delay(2000);
}

// stub: call this each loop
void controlActuators() {
  setControlVariables();
  refillTankSubcontrol();
}

void setControlVariables() {

  struct tm timeinfo;
  if (WiFi.status() != WL_CONNECTED || !getLocalTime(&timeinfo)) {
    Serial.println("No internet or time, turning on lights, and wavepump1");
    wavePump1Active = true;
    lightActive = true;
    //tuyaSetSwitch(TUYA_WAVEPUMP1_ID, wavePump1Active);
    setLightValue(lightActive);
    return;
  }

  // alternate wave pumps
  wavePump1Active = (timeinfo.tm_hour % 2 == 0);
  wavePump2Active = !wavePump1Active;

  // light schedule
  lightActive = (timeinfo.tm_hour >= lightOnHour && timeinfo.tm_hour < lightOffHour);

  // ** now call cloud‑APIs **
  setLightValue(lightActive);
  setWavepump1Value(wavePump1Active);
  setWavepump2Value(wavePump2Active);
}

void refillTankSubcontrol() {
  static unsigned long pumpStart = 0;
  unsigned long now = millis();

  if (!tankFilled) {
    // If pump just turned on, record the start time
    if (!refillPumpActive) {
      pumpStart = now;
      refillPumpActive = true;
      Serial.println("Refill Pump: ON");
      uploadRefillWaterStatusToFirestore();
    }

    // Keep pump on for up to 10 seconds
    if (now - pumpStart <= 10000) {
      digitalWrite(RELAY_FILL_PUMP, LOW);  // relay active low
      ledStatus.setWaterRefilling();
      ledStatus.update();
    } else {
      // Timeout: turn pump off
      digitalWrite(RELAY_FILL_PUMP, HIGH);
      refillPumpActive = false;
      Serial.println("Refill Pump: OFF (timeout)");
      uploadRefillWaterStatusToFirestore();
    }
  } else {
    // Tank filled: ensure pump is off and reset flag
    digitalWrite(RELAY_FILL_PUMP, HIGH);
    if (refillPumpActive) {
      refillPumpActive = false;
      Serial.println("Refill Pump: OFF (tank filled)");
      uploadRefillWaterStatusToFirestore();
    }
  }
}

// ——— Tuya helpers ———
static String makeSign(const String& clientId, uint64_t tstamp) {
  // … implement HMAC‑SHA256 of (clientId + tstamp) with TUYA_CLIENT_SECRET …
  return String(); 
}



void setLightValue(bool on) {
  if(on){
    tapoLight.on();
  } else {
    tapoLight.off();
  }
}

bool getLightValue(){
    return tapoLight.getDeviceOn();
}

void setWavepump1Value(bool on){
  tuyaSetSwitch(TUYA_WAVEPUMP1_ID, wavePump1Active);
}

bool getWavepump1Value(){
    return tuyaGetSwitch(TUYA_WAVEPUMP1_ID);
}

void setWavepump2Value(bool on){
  tuyaSetSwitch(TUYA_WAVEPUMP2_ID, wavePump2Active);
}

bool getWavepump2Value(){
   return tuyaGetSwitch(TUYA_WAVEPUMP2_ID);
}
