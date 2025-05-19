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

String makeSign(const String& clientId, const String& secret, const String& meth, const String& urlPath, uint64_t timestamp, const String& nonce = "", const String& body = "", const String& accessToken = "") {
    // SHA256 hash of the body (empty string = constant hash)
    unsigned char shaResult[32];
    const mbedtls_md_info_t* shaMd = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    mbedtls_md(shaMd, (const unsigned char*)body.c_str(), body.length(), shaResult);

    String bodyHash;
    for (int i = 0; i < 32; ++i) {
        if (shaResult[i] < 16) bodyHash += "0";
        bodyHash += String(shaResult[i], HEX);
    }

    // Convert to lowercase
    bodyHash.toLowerCase();

    // Construct the stringToSign
    String stringToSign = meth + "\n" + bodyHash + "\n\n" + urlPath;

    // Construct final sign input
    String input = clientId + accessToken + String(timestamp) + nonce + stringToSign;

    // HMAC-SHA256
    unsigned char hmacResult[32];
    mbedtls_md_hmac(shaMd,
        (const unsigned char*)secret.c_str(), secret.length(),
        (const unsigned char*)input.c_str(), input.length(),
        hmacResult);

    // Convert to uppercase hex string
    String hex;
    for (int i = 0; i < 32; i++) {
        if (hmacResult[i] < 16) hex += "0";
        hex += String(hmacResult[i], HEX);
    }

    hex.toUpperCase();
    return hex;
}





void tuyaAuthenticate() {
  uint64_t tstamp = time(nullptr) * 1000; // Current epoch time in milliseconds
  HTTPClient http;
  logPrintln("Tuya authenticating...");
  
  http.begin("https://openapi.tuyaeu.com/v1.0/token?grant_type=1");
  http.addHeader("client_id", TUYA_CLIENT_ID);
  http.addHeader("sign_method", "HMAC-SHA256");
  http.addHeader("t", String(tstamp));
  http.addHeader("sign", makeSign(TUYA_CLIENT_ID, TUYA_CLIENT_SECRET, "GET", "/v1.0/token?grant_type=1", tstamp));
  
  int code = http.GET();
  logPrint("HTTP Code: "); logPrintln(code);

  String response = http.getString();  // 🧠 Read only once
  logPrint("Response: "); logPrintln(response);

  if (code == 200) {
    DynamicJsonDocument doc(1024); // Increase size if needed
    DeserializationError error = deserializeJson(doc, response);

    if (!error) {
      tuyaToken = doc["result"]["access_token"].as<String>();
      tuyaTokenExpires = millis() + doc["result"]["expire_time"].as<uint32_t>() * 1000;

      logPrintln(tuyaToken);
      logPrintln(doc["result"]["expire_time"].as<uint32_t>());
      logPrintln("Tuya auth ok.");
    } else {
      logPrintln("JSON parse error: " + String(error.c_str()));
    }
  } else {
    logPrintln("Tuya auth err.");
  }

  http.end();
}


void tuyaSetSwitch(const char* deviceId, bool on) {
  if (millis() > tuyaTokenExpires) tuyaAuthenticate();

  uint64_t tstamp = time(nullptr) * 1000;
  String path = String("/v1.0/devices/") + deviceId + "/commands";
  String url = "https://openapi.tuyaeu.com" + path;

  // Create request body first
  DynamicJsonDocument cmd(128);
  JsonArray arr = cmd.createNestedArray("commands");
  JsonObject o = arr.createNestedObject();
  o["code"]  = "switch_1";
  o["value"] = on;

  String body;
  serializeJson(cmd, body);

  // Sign the request using method, path, timestamp, token, and body
  String sign = makeSign(TUYA_CLIENT_ID, TUYA_CLIENT_SECRET, "POST", path, tstamp, "", body, tuyaToken);

  HTTPClient http;
  logPrintln("Tuya setting switch...");
  http.begin(url);
  http.addHeader("client_id", TUYA_CLIENT_ID);
  http.addHeader("sign_method", "HMAC-SHA256");
  http.addHeader("t", String(tstamp));
  http.addHeader("sign", sign);
  http.addHeader("access_token", tuyaToken);
  http.addHeader("Content-Type", "application/json");

  int code = http.POST(body);
  logPrint("POST Code: "); logPrintln(code);
  logPrint("Response: "); logPrintln(http.getString());

  http.end();
}

bool tuyaGetSwitch(const char* deviceId) {
  if (millis() > tuyaTokenExpires) tuyaAuthenticate();

  uint64_t tstamp = time(nullptr) * 1000;
  String path = String("/v1.0/devices/") + deviceId + "/status";
  String url = "https://openapi.tuyaeu.com" + path;

  String sign = makeSign(TUYA_CLIENT_ID, TUYA_CLIENT_SECRET, "GET", path, tstamp, "", "", tuyaToken);

  HTTPClient http;
  logPrintln("Tuya getting switch...");
  http.begin(url);

  http.addHeader("client_id", TUYA_CLIENT_ID);
  http.addHeader("sign_method", "HMAC-SHA256");
  http.addHeader("t", String(tstamp));
  http.addHeader("sign", sign);
  http.addHeader("access_token", tuyaToken);

  int httpCode = http.GET();
  logPrint("HTTP Code: "); logPrintln(httpCode);

  if (httpCode == 200) {
    String payload = http.getString();
    logPrintln("Response: " + payload);

    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
      JsonArray statusArray = doc["result"].as<JsonArray>();
      for (JsonObject status : statusArray) {
        if (status["code"] == "switch_1") {
          http.end();
          return status["value"]; // true or false
        }
      }
    } else {
      logPrintln("JSON parse error in tuyaGetSwitch()");
    }
  } else {
    logPrint("Failed to GET status. HTTP code: ");
    logPrintln(httpCode);
  }

  http.end();
  return false;
}
