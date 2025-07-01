#include "actuators.h"
#include "sensors.h"
#include "led_status.h"
#include "firestore_sync.h"
#include <WiFi.h>
#include "configuration.h"
#define TAPO_DEBUG_MODE // Comment this line to disable debug messages
#include "tapo_device.h"
//#include "tuya_device.h"
#include "webserial_logging.h"

// your existing flags
bool fanActive = true;
float targetFanTemp = 24;

bool wavePump2Active = true;
bool lightActive    = true;
bool refillPumpActive = false;

bool forceModeActive = false;

// light schedule
int lightOnHour  = 17;
int lightOffHour = 23;

unsigned long pumpLastFilled;

TapoDevice tapoLight;
TapoDevice tapoRefillPump;

void setupActuators() {
  pinMode(RELAY_FILL_PUMP, OUTPUT);
  digitalWrite(RELAY_FILL_PUMP, true);
  tapoLight.begin(TAPO_LIGHT_IP, TAPO_USERNAME, TAPO_PASSWORD);
  tapoRefillPump.begin(TAPO_REFILL_PUMP_IP, TAPO_USERNAME, TAPO_PASSWORD);
  lightActive = getLightValue();
  refillPumpActive = getRefillPumpValue();
  setLightValue(lightActive);
  setRefillPumpValue(refillPumpActive);
  //wavePump2Active = getWavepump2Value();
  logPrintln("Actuators initialized.");

  prefs.begin(CONFIGS_PREFS_NAMESPACE, false);
  lightOnHour = prefs.getInt(CONFIG_LIGHT_ON_KEY, 17);
  lightOffHour = prefs.getInt(CONFIG_LIGHT_OFF_KEY, 23);
  prefs.end();
  
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
    logPrintln("No internet or time, turning on lights, and fan");
    fanActive = true;
    wavePump2Active = false;
    lightActive = true;
    setLightValue(lightActive);
    setRefillPumpValue(refillPumpActive);
    //setWavepump2Value(wavePump2Active);
    return;
  }

  if(!forceModeActive){
    // light schedule
    lightActive = (timeinfo.tm_hour >= lightOnHour && timeinfo.tm_hour < lightOffHour);
//
//    if(tempC <= (targetFanTemp - 0.1)){
//      fanActive = false;
//    }
//    if(tempC >= (targetFanTemp + 0.1)){
//      fanActive = true;
//    }
    
  } else {
    logPrintln("Force mode active: skipping set control variables");
  }

  // ** now call cloud‑APIs **
  setLightValue(lightActive);
  setRefillPumpValue(refillPumpActive);
  //setWavepump2Value(wavePump2Active);
}

void refillTankSubcontrol() {
  unsigned long pumpStart = millis();
  bool timeout = false;

  if(forceModeActive) {
    logPrintln("Force mode active: skipping refill tank");
  }

  if(tankFilled){
    return;
  } else {
    logPrintln("Tank NOT fully filled");
  }

  if(pumpLastFilled && millis() - pumpLastFilled < TANK_WATER_FILL_TIME_WINDOW){
    logPrintln("Filled less than TANK_WATER_FILL_TIME_WINDOW ago");
    return;
  }

  refillPumpActive = true;
  logPrintln("Refill Pump: ON");
  uploadRefillWaterStatusToFirestore();
  
  while (!timeout) { 
    // Keep pump on for up to TANK_WATER_FILL_TIME_TO
    if (millis() - pumpStart <= TANK_WATER_FILL_TIME_TO) {
      pumpLastFilled = millis();
      setRefillPumpValue(refillPumpActive);
      ledStatus.setWaterRefilling();
      ledStatus.update();
      readAllSensors();
      delay(5000);
    } else if(timeout || tankFilled) {
      ledStatus.off();
      ledStatus.update();
      logPrintln("Refill Pump TIMEMOUT");
      timeout = true;
    }
  }
  refillPumpActive = false;
  setRefillPumpValue(refillPumpActive);
  logPrintln("Refill Pump: OFF");

  uploadRefillWaterStatusToFirestore();
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

void setRefillPumpValue(bool on){
  if(on){
    tapoRefillPump.on();
  } else {
    tapoRefillPump.off();
  }
}

bool getRefillPumpValue(){
    return tapoRefillPump.getDeviceOn();
}

//void setWavepump2Value(bool on){
//  tuyaSetSwitch(TUYA_WAVEPUMP2_ID, wavePump2Active);
//}
//
//bool getWavepump2Value(){
//   return tuyaGetSwitch(TUYA_WAVEPUMP2_ID);
//}
