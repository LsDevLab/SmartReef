#include "actuators.h"
#include "sensors.h"
#include "led_status.h"
#include "firestore_sync.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "configuration.h"
#define TAPO_DEBUG_MODE // Comment this line to disable debug messages
#include "tapo_device.h"

static String tuyaToken;
static unsigned long tuyaTokenExpires = 0;
static String tapoToken;

// your existing flags
bool wavePump1Active = true;
bool wavePump2Active = true;
bool lightActive    = true;
bool refillPumpActive = false;

// light schedule
int lightOnHour  = 17;
int lightOffHour = 24;

TapoDevice tapo;


void setupActuators() {
  pinMode(RELAY_FILL_PUMP, OUTPUT);
  digitalWrite(RELAY_FILL_PUMP, !refillPumpActive);
  Serial.println("Refill pump control initialized.");
  delay(2000);
}

// stub: call this each loop
void controlActuators() {
  setControlVariables();
  refillTankSubcontrol();
}

void setControlVariables() {
  if (WiFi.status() != WL_CONNECTED) return;

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Time not available");
    return;
  }

  // alternate wave pumps
  wavePump1Active = (timeinfo.tm_hour % 2 == 0);
  wavePump2Active = !wavePump1Active;

  // light schedule
  lightActive = (timeinfo.tm_hour >= lightOnHour && timeinfo.tm_hour < lightOffHour);

  // ** now call cloud‑APIs **
  //tuyaSetSwitch(TUYA_DEVICE_ID_1, wavePump1Active);
  //tuyaSetSwitch(TUYA_DEVICE_ID_2, wavePump2Active);
  tapoControl(TAPO_DEVICE_IP, lightActive);
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

void tuyaAuthenticate() {
  uint64_t tstamp = millis();
  HTTPClient http;
  http.begin("https://openapi.tuyacn.com/v1.0/token?grant_type=1");
  http.addHeader("client_id", TUYA_CLIENT_ID);
  http.addHeader("sign_method","HMAC-SHA256");
  http.addHeader("sign", makeSign(TUYA_CLIENT_ID, tstamp));
  int code = http.POST("");
  if (code==200) {
    DynamicJsonDocument doc(512);
    deserializeJson(doc, http.getString());
    tuyaToken = doc["result"]["access_token"].as<String>();
    tuyaTokenExpires = millis() + doc["result"]["expire_time"].as<uint32_t>()*1000;
  }
  http.end();
}

void tuyaSetSwitch(const char* deviceId, bool on) {
  if (millis() > tuyaTokenExpires) tuyaAuthenticate();

  HTTPClient http;
  String url = String("https://openapi.tuyacn.com/v1.0/devices/") + deviceId + "/commands";
  http.begin(url);
  http.addHeader("Authorization", tuyaToken);
  http.addHeader("Content-Type","application/json");

  DynamicJsonDocument cmd(128);
  JsonArray arr = cmd.createNestedArray("commands");
  JsonObject o = arr.createNestedObject();
  o["code"]  = "switch_1";
  o["value"] = on;

  String body;
  serializeJson(cmd, body);
  http.POST(body);
  http.end();
}


void tapoControl(const char* deviceIP, bool on) {
  tapo.begin(deviceIP, TAPO_USERNAME, TAPO_PASSWORD);
  if(on){
    tapo.on();
  } else {
    tapo.off();
  }
}
