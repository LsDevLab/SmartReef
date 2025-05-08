#include "tuya_device.h"
#include <Arduino.h>
#include <WiFi.h>
#include "configuration.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <mbedtls/md.h>
#include "webserial_logging.h"

static String tuyaToken;
static unsigned long tuyaTokenExpires = 0;
static String tapoToken;

// If using an access token in the signature
// leave as "" if not needed
String makeSign(const String& clientId, uint64_t timestamp, const String& clientSecret, const String& accessToken = "") {
    String payload = clientId + accessToken + String(timestamp);

    const size_t keyLength = clientSecret.length();
    const size_t payloadLength = payload.length();

    unsigned char hmacResult[32]; // SHA256 = 32 bytes
    const mbedtls_md_info_t* md = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);

    mbedtls_md_hmac(md,
                    (const unsigned char*)clientSecret.c_str(), keyLength,
                    (const unsigned char*)payload.c_str(), payloadLength,
                    hmacResult);

    // Convert to hex string
    String hex;
    for (int i = 0; i < 32; i++) {
        if (hmacResult[i] < 16) hex += "0";
        hex += String(hmacResult[i], HEX);
    }

    hex.toLowerCase(); // Optional, depends on API requirement
    return hex;
}



void tuyaAuthenticate() {
  uint64_t tstamp = millis();
  HTTPClient http;
  http.begin("https://openapi.tuyacn.com/v1.0/token?grant_type=1");
  http.addHeader("client_id", TUYA_CLIENT_ID);
  http.addHeader("sign_method","HMAC-SHA256");
  http.addHeader("sign", makeSign(TUYA_CLIENT_ID, tstamp, TUYA_CLIENT_SECRET, tuyaToken));
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

bool tuyaGetSwitch(const char* deviceId){
    if (millis() > tuyaTokenExpires) tuyaAuthenticate();

    HTTPClient http;
    String url = String("https://openapi.tuyacn.com/v1.0/devices/") + deviceId + "/status";
    http.begin(url);
    http.addHeader("Authorization", tuyaToken);

    int httpCode = http.GET();
    if (httpCode == 200) {
        String payload = http.getString();
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, payload);
        if (!error) {
            JsonArray statusArray = doc["result"].as<JsonArray>();
            for (JsonObject status : statusArray) {
                const char* code = status["code"];
                if (String(code) == "switch_1") {
                    return status["value"]; // returns true or false
                }
            }
        } else {
            logPrintln("JSON parse error in getTuyaDeviceSwitchState()");
        }
    } else {
        logPrint("Failed to GET status. HTTP code: ");
        logPrintln(httpCode);
    }

    http.end();
    return false; // Default if not found or error
}
