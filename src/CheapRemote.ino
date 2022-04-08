/**
 * GoPro recording status and battery percent by Gryphus21.
 * CheapRemote v0.1
 */

#include <M5StickCPlus.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char SSID[] = "GP28532818";
const char PASSWORD[] = "q+6-J5h-WjS";
const String PATH_STATUS = "/gp/gpControl/status";

IPAddress goproIP(10, 5, 5, 9);
IPAddress localIP(10, 5, 5, 100);
IPAddress gatewayIP(10, 5, 5, 9);
IPAddress subnetMask(255, 255, 255, 0);
IPAddress dns0(1, 1, 1, 1);
IPAddress dns1(1, 0, 0, 1);



bool tryToConnect() {
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setCursor(0, 120);
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.print("Connecting...");

    if (wifiInitConnection()) {
        M5.Lcd.fillScreen(TFT_BLACK);
        M5.Lcd.setCursor(0, 120);
        M5.Lcd.setTextColor(TFT_GREEN);
        M5.Lcd.print("Connected:" + WiFi.localIP().toString());

        Serial.println("[WiFi tryToConnect] [OK]Connected\n");
        return true;
    } else {
        M5.Lcd.fillScreen(TFT_BLACK);
        M5.Lcd.setCursor(0, 120);
        M5.Lcd.setTextColor(TFT_RED);
        M5.Lcd.print("Connection failed");

        Serial.println("[WiFi tryToConnect] [NOPE]Connection failed\n");
        return false;
    }
}

bool wifiCheckConnection() {
    return WiFi.status() == WL_CONNECTED;
}

bool wifiInitConnection() {
    int attempt = 0;

    bool wifiConfig = WiFi.config(localIP, gatewayIP, subnetMask, dns0, dns1);
    if (wifiConfig)
        Serial.println("[WiFi wifiInitConnection] Configuration: OK");
    else
        Serial.println("[WiFi wifiInitConnection] Configuration: NOPE");

    bool wifiBegin = WiFi.begin(SSID, PASSWORD);
    if (wifiBegin)
        Serial.println("[WiFi wifiInitConnection] Wifi begin OK");
    else
        Serial.println("[WiFi wifiInitConnection] Wifi begin NOPE");

    Serial.println("[WiFi wifiInitConnection] Connectiong...");
    Serial.println("[WiFi wifiInitConnection] SSID:" + WiFi.SSID());
    Serial.println("[WiFi wifiInitConnection] PSK:" + WiFi.psk());

    while (!wifiCheckConnection()) {
        if (attempt >= 10)
            return false;
        attempt++;
        delay(500);
    }
    return true;
}

void wifiDisconnect(bool wifiOff = false, bool eraseAP = false) {
    bool wifiDisconnect = WiFi.disconnect(wifiOff, eraseAP);

    if (wifiDisconnect)
        Serial.println("[WiFi wifiDisconnect] OK");
    else
        Serial.println("[WiFi wifiDisconnect] NOPE");

    M5.Lcd.setTextColor(TFT_ORANGE);
    M5.Lcd.print("Disconnected");
}

String goProRequest(String uri) {
    Serial.println("\n### goProRequest() ###");
    HTTPClient http;
    String payload;
    int httpStatusCode;

    http.begin(goproIP.toString(), 80, uri);
    //http.addHeader("Connection", "close");
    http.addHeader("Host", goproIP.toString());
    http.addHeader("Accept-Encoding", "gzip, deflate");
    http.addHeader("User-Agent", "m5/1.0");

    Serial.println("http begin + headers");

    httpStatusCode = http.GET();
    Serial.println("HTTP STATUS CODE: " + String(httpStatusCode));

    if (httpStatusCode > 0) {
        if (httpStatusCode == (int) HTTP_CODE_OK) {
            payload = http.getString();
        }
    } else {
        Serial.println("Error: " + http.errorToString(httpStatusCode));
    }

    http.end();
    Serial.println("http end");
    Serial.println("RESPONSE:" + payload + "-END-");
    Serial.println("### END goProRequest() ###\n\n");
    return payload;
}

String batteryToText(int code) {
    switch (code) {
    case 0:
        return "Empty";
        break;

    case 1:
        return "Low";
        break;

    case 2:
        return "Medium";
        break;

    case 3:
        return "Full";
        break;

    case 4:
        return "Charging";
        break;

    default:
        return "-ERR-";
    }
}

void setup() {
    M5.begin(true, true, true); // lcd, power, serial

    M5.Lcd.setRotation(3);
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.setTextSize(2);

    Serial.println("[SETUP] Wifi mode STA");
    WiFi.mode(WIFI_STA);
    Serial.println("[SETUP] Disconnected from GoPro\n");
    wifiDisconnect();
    delay(1000);
}

void loop() {
    Serial.println("\n[LOOP --> WiFi checkConnection]");

    if (wifiCheckConnection()) {
        Serial.println("[WiFi tryToConnect] M5 is connected !");
        Serial.println("[WiFi tryToConnect] Connection details:");
        Serial.println(" -HOST IP: " + WiFi.localIP().toString());
        Serial.println(" -GATEWAY IP: " + WiFi.gatewayIP().toString());
        Serial.println(" -SUBNET MASK: " + WiFi.subnetMask().toString());
        Serial.println(" -DNS 0: " + WiFi.dnsIP(0).toString());
        Serial.println(" -DNS 1: " + WiFi.dnsIP(1).toString());
        Serial.println(" -BSSID (mac addr): " + WiFi.BSSIDstr());
        Serial.println(" -RSSI: " + String(WiFi.RSSI()));
        Serial.println("\n");

        M5.Lcd.fillScreen(TFT_BLACK);
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.setTextColor(TFT_WHITE);
        M5.Lcd.print("Send request...");

        Serial.println("[LOOP][Request] Send request...");
        String goProJson = goProRequest(PATH_STATUS);
        Serial.println("[LOOP][Request] request sent, response:\n" + goProJson + "-END-");

        if (goProJson != "") {
            Serial.println("[LOOP][JSON] response analysis");
            StaticJsonDocument < 4096 > jsonDoc;

            DeserializationError deserializationError = deserializeJson(jsonDoc, goProJson);
            if (deserializationError) {
                M5.Lcd.fillScreen(TFT_BLACK);
                M5.Lcd.setCursor(0, 0);
                M5.Lcd.setTextColor(TFT_RED);
                M5.Lcd.print("JSON wrong");

                Serial.println("[LOOP][JSON] Deserialization failed !");

                delay(1000);
                return;
            }
            Serial.println("[LOOP][JSON] Deserialization complete");

            String recordingStatus = jsonDoc["status"]["8"];
            String batteryStatus = batteryToText(jsonDoc["status"]["2"]);
            int batteryPercent = jsonDoc["status"]["70"];
            bool batteryIsPlugged = jsonDoc["status"]["1"];

            Serial.println("[LOOP][JSON] recording status:" + recordingStatus);
            Serial.println("[LOOP][JSON] battery status:" + batteryStatus);
            Serial.println("[LOOP][JSON] battery percent:" + String(batteryPercent));
            Serial.println("[LOOP][JSON] battery isPlugged:" + String(batteryIsPlugged));

            M5.Lcd.fillScreen(TFT_BLACK);
            M5.Lcd.setCursor(0, 20);
            if (recordingStatus == "1") {
                M5.Led.on();
                M5.Lcd.setTextColor(TFT_GREEN);
            } else {
                M5.Led.off();
                M5.Lcd.setTextColor(TFT_RED);
            }
            M5.Lcd.print("REC: " + recordingStatus);

            M5.Lcd.setCursor(0, 50);
            M5.Lcd.setTextColor(TFT_WHITE);
            M5.Lcd.print("Battery percent: " + batteryStatus + " (" + String(batteryPercent) + "%)");

            M5.Lcd.setCursor(0, 70);
            M5.Lcd.setTextColor(TFT_WHITE);
            M5.Lcd.print("Battery in pluged: " + String(batteryIsPlugged ? "Yes" : "No"));

            //delay(500);
            //wifiDisconnect(0);
        } else {
            M5.Lcd.fillScreen(TFT_BLACK);
            M5.Lcd.setCursor(0, 20);
            M5.Lcd.setTextColor(TFT_RED);
            M5.Lcd.print("GoPro not responding...");

            Serial.println("[LOOP][Request] Response is empty");

            //delay(500);
            //wifiDisconnect(0);
        }
    } else {
        Serial.println("[LOOP][WiFi] It is not connected, I try to connect...\n\n");
        tryToConnect();
    }

    /*if (M5.Axp.GetBtnPress() == 1) {
        M5.Lcd.fillScreen(TFT_WHITE);
        WiFi.disconnect();
        while(1);
    }*/
    delay(1000);
}
