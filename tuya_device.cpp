#include "tuya_device.h"
#include <Arduino.h>
#include <WiFi.h>
#include "configuration.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <mbedtls/md.h>
#include "webserial_logging.h"
#include <esp_heap_caps.h>

static String tuyaToken;
static unsigned long tuyaTokenExpires = 0;
static String tapoToken;
static HTTPClient http;
static bool httpClientBusy = false;


void logHeapStats() {
  logPrint("Free heap: ");
  logPrintln(String(ESP.getFreeHeap()) + " bytes");

  logPrint("Minimum ever free heap: ");
  logPrintln(String(ESP.getMinFreeHeap()) + " bytes");

  logPrint("Free internal heap: ");
  logPrintln(String(heap_caps_get_free_size(MALLOC_CAP_INTERNAL)) + " bytes");
}

String makeSign(const String& clientId, const String& secret, const String& meth, const String& urlPath, uint64_t timestamp, const String& nonce = "", const String& body = "", const String& accessToken = "") {
    unsigned char shaResult[32];
    const mbedtls_md_info_t* shaMd = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    mbedtls_md(shaMd, (const unsigned char*)body.c_str(), body.length(), shaResult);

    String bodyHash;
    bodyHash.reserve(65);
    for (int i = 0; i < 32; ++i) {
        if (shaResult[i] < 16) bodyHash += "0";
        bodyHash += String(shaResult[i], HEX);
    }
    bodyHash.toLowerCase();

    String stringToSign;
    stringToSign.reserve(256);
    stringToSign = meth + "\n" + bodyHash + "\n\n" + urlPath;

    String input;
    input.reserve(256);
    input = clientId + accessToken + String(timestamp) + nonce + stringToSign;

    unsigned char hmacResult[32];
    mbedtls_md_hmac(shaMd,
        (const unsigned char*)secret.c_str(), secret.length(),
        (const unsigned char*)input.c_str(), input.length(),
        hmacResult);

    String hex;
    hex.reserve(65);
    for (int i = 0; i < 32; i++) {
        if (hmacResult[i] < 16) hex += "0";
        hex += String(hmacResult[i], HEX);
    }

    hex.toUpperCase();
    return hex;
}

void tuyaAuthenticate() {
  logPrintln("Before Tuya: heap usage");
  logHeapStats();

if (httpClientBusy) {
  logPrintln("HTTPClient is busy, skipping.");
  return;
}
httpClientBusy = true;
  
  uint64_t tstamp = time(nullptr) * 1000;
  logPrintln("Tuya authenticating...");

  http.begin("https://openapi.tuyaeu.com/v1.0/token?grant_type=1");
  http.addHeader("client_id", TUYA_CLIENT_ID);
  http.addHeader("sign_method", "HMAC-SHA256");
  http.addHeader("t", String(tstamp));
  http.addHeader("sign", makeSign(TUYA_CLIENT_ID, TUYA_CLIENT_SECRET, "GET", "/v1.0/token?grant_type=1", tstamp));

  int code = http.GET();
  logPrint("HTTP Code: "); logPrintln(code);

  String response = http.getString();
  logPrint("Response: "); logPrintln(response);

  if (code == 200) {
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, response);

    if (!error) {
      tuyaToken = doc["result"]["access_token"].as<String>();
      tuyaTokenExpires = millis() + doc["result"]["expire_time"].as<uint32_t>() * 1000;
      logPrintln("Tuya auth ok.");
    } else {
      logPrintln("JSON parse error: " + String(error.c_str()));
    }
  } else {
    logPrintln("Tuya auth err.");
  }
  http.end();

httpClientBusy = false;

  
  logPrintln("After Tuya: heap usage");
  logHeapStats();
}

void tuyaSetSwitch(const char* deviceId, bool on) {

if (httpClientBusy) {
  logPrintln("HTTPClient is busy, skipping.");
  return;
}
httpClientBusy = true;
  
  if (millis() > tuyaTokenExpires) tuyaAuthenticate();

  logPrintln("Before Tuya setSwitch: heap usage");
  logHeapStats();

  uint64_t tstamp = time(nullptr) * 1000;

  String path;
  path.reserve(64);
  path = "/v1.0/devices/";
  path += deviceId;
  path += "/commands";

  String url;
  url.reserve(96);
  url = "https://openapi.tuyaeu.com";
  url += path;

  DynamicJsonDocument cmd(512);
  JsonArray arr = cmd.createNestedArray("commands");
  JsonObject o = arr.createNestedObject();
  o["code"]  = "switch_1";
  o["value"] = on;

  String body;
  serializeJson(cmd, body);

  String sign = makeSign(TUYA_CLIENT_ID, TUYA_CLIENT_SECRET, "POST", path, tstamp, "", body, tuyaToken);

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

httpClientBusy = false;

  logPrintln("After Tuya setSwitch: heap usage");
  logHeapStats();
}

bool tuyaGetSwitch(const char* deviceId) {
  if (millis() > tuyaTokenExpires) tuyaAuthenticate();

  if (httpClientBusy) {
  logPrintln("HTTPClient is busy, skipping.");
  return false;
}
httpClientBusy = true;

  uint64_t tstamp = time(nullptr) * 1000;

  String path;
  path.reserve(64);
  path = "/v1.0/devices/";
  path += deviceId;
  path += "/status";

  String url;
  url.reserve(96);
  url = "https://openapi.tuyaeu.com";
  url += path;

  String sign = makeSign(TUYA_CLIENT_ID, TUYA_CLIENT_SECRET, "GET", path, tstamp, "", "", tuyaToken);

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
          httpClientBusy = false;
          return status["value"];
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
  httpClientBusy = false;

  return false;
}
