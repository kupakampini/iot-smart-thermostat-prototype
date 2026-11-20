#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "YOUR_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "Smart Thermostat"
#define BLYNK_AUTH_TOKEN "YOUR_BLYNK_AUTH_TOKEN"

#include <SPI.h>
#include <WiFiNINA.h>
#include <BlynkSimpleWiFiNINA.h>

char ssid[] = "YOUR_WIFI_SSID";
char pass[] = "YOUR_WIFI_PASSWORD";

// Helper function to extract values from "TELEM|TEMP:25.4|HUM:60.1|..."
String getValue(String data, String key) {
  int startIdx = data.indexOf(key);
  if (startIdx == -1) return "";
  startIdx += key.length();
  int endIdx = data.indexOf('|', startIdx);
  if (endIdx == -1) endIdx = data.length();
  return data.substring(startIdx, endIdx);
}

// Blynk V1: Relay Switch -> Forward command to Uno
BLYNK_WRITE(V1) {
  int val = param.asInt();
  Serial1.println(val == 1 ? "CMD:RELAY:1" : "CMD:RELAY:0");
}

// Blynk V2: Threshold Slider -> Forward command to Uno
BLYNK_WRITE(V2) {
  float thresh = param.asFloat();
  Serial1.println("CMD:THRESH:" + String(thresh, 1));
}

// Blynk V3: Auto Mode Switch -> Forward command to Uno
BLYNK_WRITE(V3) {
  int mode = param.asInt();
  Serial1.println(mode == 1 ? "CMD:MODE:AUTO" : "CMD:MODE:MANUAL");
}

void parseUnoTelemetry(String msg) {
  msg.trim();
  if (!msg.startsWith("TELEM|")) return;

  String tempStr   = getValue(msg, "TEMP:");
  String humStr    = getValue(msg, "HUM:");
  String threshStr = getValue(msg, "THRESH:");
  String relayStr  = getValue(msg, "RELAY:");
  String modeStr   = getValue(msg, "MODE:");

  if (tempStr != "")   Blynk.virtualWrite(V0, tempStr.toFloat());
  if (relayStr != "")  Blynk.virtualWrite(V1, relayStr.toInt());
  if (threshStr != "") Blynk.virtualWrite(V2, threshStr.toFloat());
  if (modeStr != "")   Blynk.virtualWrite(V3, modeStr == "AUTO" ? 1 : 0);
  if (humStr != "")    Blynk.virtualWrite(V4, humStr.toFloat());
}

void setup() {
  Serial.begin(115200);   // Optional debug monitor on MKR1010
  Serial1.begin(9600);    // Hardware UART (Pins 13 RX, 14 TX) to Uno

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
}

void loop() {
  Blynk.run();

  if (Serial1.available()) {
    String incoming = Serial1.readStringUntil('\n');
    Serial.println("From Uno: " + incoming); // Debug output
    parseUnoTelemetry(incoming);
  }
}
