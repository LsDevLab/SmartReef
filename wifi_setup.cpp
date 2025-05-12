#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include "led_status.h"
#include "wifi_setup.h"
#include "web_server.h"
#include "configuration.h"
#include "webserial_logging.h"

Preferences prefs;
WebServer configServer(80);

bool shouldSaveConfig = false;

void checkNetwork() {
  if(WiFi.status() != WL_CONNECTED){
    setupNetwork();
  } else {
    ledStatus.setStationConnected();
    unsigned long startAttempt = millis();
    while(millis()-startAttempt < 1300){
      ledStatus.update();
    }
    ledStatus.off();
    ledStatus.update();
  }
}

// ======== SETUP ========
void setupNetwork() {
  
  prefs.begin(WIFI_PREFS_NAMESPACE, false);

  String savedSSID = prefs.getString(SSID_KEY, "");
  String savedPASS = prefs.getString(PASS_KEY, "");

  if (savedSSID != "") {

    wl_status_t status = WiFi.status();
    if (status != WL_CONNECTED && status != WL_IDLE_STATUS && status != WL_SCAN_COMPLETED) {
        WiFi.begin(savedSSID.c_str(), savedPASS.c_str());
    }


    logPrint("Trying to connect to WiFi network ");
    logPrint(savedSSID.c_str());
    logPrintln("...");
    
    ledStatus.setStationConnecting();
    unsigned long startAttempt = millis();
    while ((WiFi.status() != WL_CONNECTED || millis() - startAttempt < 3000) && millis() - startAttempt < 20000) {
      ledStatus.update();
    }

      startAttempt = millis();
      ledStatus.off();
      while(millis()-startAttempt < 700){
        ledStatus.update();
      }

    if (WiFi.status() == WL_CONNECTED) {

      logPrintln("Connected as STA.");
      logPrint("IP: ");
      logPrintln(WiFi.localIP().toString());
     
      ledStatus.off();
      ledStatus.update();
      prefs.end();

      Serial.printf("Wi-Fi RSSI: %d dBm\n", WiFi.RSSI());


      startWebServer();

      return;
    }

    logPrintln("Cannot connect to Wifi.");
    ledStatus.setErrorStationConnecting();
    startAttempt = millis();
    while(millis()-startAttempt < 4000){
      ledStatus.update();
    }
    ledStatus.off();
    ledStatus.update();

    WiFi.disconnect();
    
    prefs.end();
    return;
    
  }

  // If no saved credentials or connect failed
  logPrintln("Starting AP mode for configuration...");
  startAPMode();
  prefs.end();
}


// ======== AP MODE SETUP ========
void startAPMode() {

    logPrintln("Entering startAPMode()...");

  //ledStatus.setAPMode();  // Could be crashing here!
  logPrintln("ledStatus.setAPMode() done.");

  WiFi.disconnect(true);
  logPrintln("WiFi.disconnect() done.");

  WiFi.mode(WIFI_OFF);
  logPrintln("WiFi.mode(WIFI_OFF) done.");

  delay(100);
  logPrintln("Delay after WiFi.mode(WIFI_OFF) complete.");

  WiFi.mode(WIFI_AP);
  logPrintln("WiFi.mode(WIFI_AP) done.");

  bool apStarted = WiFi.softAP(CONFIG_AP_SSID, CONFIG_AP_PASSWORD);
  if (!apStarted) {
    logPrintln("WiFi.softAP() failed!");
    return;
  }
  logPrintln("WiFi.softAP() success.");

  IPAddress IP = WiFi.softAPIP();
  logPrint("AP IP address: ");
  logPrintln(IP);

configServer.on("/", HTTP_GET, []() {
  configServer.send(200, "text/html", R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <style>
         body {
          font-family: Arial, sans-serif;
          background: #d0f0f7; /* Light aqua blue */
          margin: 0;
          padding: 0;
          display: flex;
          justify-content: center;
          align-items: center;
          height: 100vh;
        }
        .container {
          background: #ffffff;
          padding: 24px;
          border-radius: 15px;
          box-shadow: 0 6px 15px rgba(0,0,0,0.1);
          max-width: 90%;
          width: 320px;
        }
        h2 {
          text-align: center;
          color: #007B8A; /* Aqua blue */
        }
        p {
          text-align: center;
          color: #666;
          font-size: 11px;
        }
        input[type="text"],
        input[type="password"] {
          width: 100%;
          padding: 12px;
          margin: 10px 0;
          border: 1px solid #aad1da;
          border-radius: 8px;
          background-color: #f0fcff;
        }
        input[type="submit"] {
          width: 100%;
          background-color: #00a3af; /* Aqua-teal */
          color: white;
          padding: 14px;
          border: none;
          border-radius: 8px;
          font-size: 16px;
          cursor: pointer;
        }
        input[type="submit"]:hover {
          background-color: #008896;
        }
      </style>
    </head>
    <body>
      <div class="container">
        <h2>SmartReef</h2>
        <p>Enter router credentials</p>
        <form action="/save" method="POST">
          SSID: <input name="ssid" type="text" required><br>
          Password: <input name="pass" type="password"><br>
          <input type="submit" value="Save">
        </form>
      </div>
    </body>
    </html>
  )rawliteral");
});

  configServer.on("/save", HTTP_POST, []() {
    String ssid = configServer.arg("ssid");
    String pass = configServer.arg("pass");

    prefs.begin(WIFI_PREFS_NAMESPACE, false);
    prefs.putString(SSID_KEY, ssid);
    prefs.putString(PASS_KEY, pass);
    prefs.end();

    configServer.send(200, "text/html", R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <style>
        body {
          font-family: Arial, sans-serif;
          background: #d0f0f7; /* Light aqua blue */
          margin: 0;
          padding: 0;
          display: flex;
          justify-content: center;
          align-items: center;
          height: 100vh;
        }
        .container {
          background: #ffffff;
          padding: 30px;
          border-radius: 15px;
          box-shadow: 0 6px 15px rgba(0,0,0,0.1);
          max-width: 90%;
          width: 350px;
          text-align: center;
        }
        h2 {
          color: #007B8A; /* Aqua blue */
        }
        p {
          color: #444;
          font-size: 16px;
        }
        .btn {
          display: inline-block;
          background-color: #00a3af;
          color: white;
          padding: 12px 20px;
          border-radius: 8px;
          font-size: 16px;
          text-decoration: none;
          cursor: pointer;
        }
        .btn:hover {
          background-color: #008896;
        }
      </style>
    </head>
    <body>
      <div class="container">
        <h2>Settings Saved!</h2>
        <p>Your WiFi settings have been successfully saved. The device will reboot now.</p>
        <a href="/" class="btn">Go Back to Settings</a>
      </div>
    </body>
    </html>
  )rawliteral");

   delay(2000);
    ESP.restart();
  });

  configServer.begin();

  while(true) {
    logPrintln(WiFi.status());
    ledStatus.update();       // Fast blink for AP mode
    configServer.handleClient();
    delay(10);
  }
  
}
