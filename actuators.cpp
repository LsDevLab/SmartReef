#include "actuators.h"
#include "sensors.h"
#include "led_status.h"
#include "firestore_sync.h"
#include <WiFi.h>
#include "configuration.h"
#define TAPO_DEBUG_MODE // Comment this line to disable debug messages
#include "tapo_device.h"
#include "tuya_device.h"
#include "webserial_logging.h"


// your existing flags
bool wavePump1Active = true;
bool wavePump2Active = true;
bool lightActive    = true;
bool refillPumpActive = false;

// light schedule
int lightOnHour  = 17;
int lightOffHour = 23;

unsigned long pumpLastFilled;

TapoDevice tapoLight;

void setupActuators() {
  pinMode(RELAY_FILL_PUMP, OUTPUT);
  digitalWrite(RELAY_FILL_PUMP, !refillPumpActive);
  tapoLight.begin(TAPO_LIGHT_IP, TAPO_USERNAME, TAPO_PASSWORD);
  lightActive = getLightValue();
  //wavePump1Active = getWavepump1Value();
  //wavePump2Active = getWavepump2Value();
  logPrintln("Refill pump control initialized.");
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
    logPrintln("No internet or time, turning on lights, and wavepump1");
    wavePump1Active = true;
    wavePump2Active = false;
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
  unsigned long pumpStart = millis();
  bool timeout = false;

  readTankFilledSensor();
  if(pumpLastFilled && millis() - pumpLastFilled < 24*60*60000){
      logPrintln("Filled less than 1 day ago");
      return;
    }
  while (!timeout && !tankFilled) { // max 1 a day
    refillPumpActive = true;
    logPrintln("Refill Pump: ON");
    // Keep pump on for up to 10 seconds
    if (millis() - pumpStart <= 10000) {
      pumpLastFilled = millis();
      digitalWrite(RELAY_FILL_PUMP, LOW);  // relay active low
      ledStatus.setWaterRefilling();
      ledStatus.update();
      delay(1000);
    } else {
      ledStatus.off();
      ledStatus.update();
      logPrintln("Refill Pump TIMEOUT");
      timeout = true;
    }
    readTankFilledSensor();
    //uploadRefillWaterStatusToFirestore(); too slow
  }
  digitalWrite(RELAY_FILL_PUMP, HIGH);
  refillPumpActive = false;
  logPrintln("Refill Pump: OFF (tank filled)");
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
