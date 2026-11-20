#include <DHT.h>
#include <SoftwareSerial.h>

#define DHTPIN 2
#define DHTTYPE DHT22
const int RELAY_PIN = 4;
const bool RELAY_ACTIVE_LOW = false; // Change to true if relay triggers on LOW

DHT dht(DHTPIN, DHTTYPE);
SoftwareSerial mkrSerial(10, 11); // RX = D10, TX = D11

float currentTemp = 0.0;
float currentHum = 0.0;
float targetThreshold = 25.0;
bool relayState = false;
bool autoMode = true;

unsigned long lastRead = 0;
const unsigned long INTERVAL = 2500;

void setRelay(bool state) {
  relayState = state;
  if (RELAY_ACTIVE_LOW) {
    digitalWrite(RELAY_PIN, relayState ? LOW : HIGH);
  } else {
    digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
  }
}

// Processes commands from either PC USB or MKR1010 (Blynk)
void processCommand(String cmd, Stream &replyPort) {
  cmd.trim();
  if (cmd.length() == 0) return;

  if (cmd == "CMD:MODE:AUTO") {
    autoMode = true;
    replyPort.println("ACK:MODE:AUTO");
  } 
  else if (cmd == "CMD:MODE:MANUAL") {
    autoMode = false;
    replyPort.println("ACK:MODE:MANUAL");
  } 
  else if (cmd == "CMD:RELAY:1") {
    autoMode = false;
    setRelay(true);
    replyPort.println("ACK:RELAY:1");
  } 
  else if (cmd == "CMD:RELAY:0") {
    autoMode = false;
    setRelay(false);
    replyPort.println("ACK:RELAY:0");
  } 
  else if (cmd.startsWith("CMD:THRESH:")) {
    float val = cmd.substring(11).toFloat();
    if (val >= 5.0 && val <= 45.0) {
      targetThreshold = val;
      replyPort.println("ACK:THRESH:" + String(targetThreshold, 1));
    } else {
      replyPort.println("ERR:RANGE");
    }
  } 
  else {
    replyPort.println("ERR:INVALID_CMD");
  }
}

void setup() {
  Serial.begin(115200);    // PC USB Serial
  mkrSerial.begin(9600);   // Reliable baud rate for SoftwareSerial
  dht.begin();

  pinMode(RELAY_PIN, OUTPUT);
  setRelay(false);
}

void loop() {
  // 1. Check commands from PC USB
  if (Serial.available()) {
    String pcCmd = Serial.readStringUntil('\n');
    processCommand(pcCmd, Serial);
  }

  // 2. Check commands from MKR1010 (Blynk)
  if (mkrSerial.available()) {
    String mkrCmd = mkrSerial.readStringUntil('\n');
    processCommand(mkrCmd, mkrSerial);
  }

  // 3. Periodic Sensing & Thermostat Control
  if (millis() - lastRead >= INTERVAL) {
    lastRead = millis();

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t)) {
      Serial.println("ERR:DHT_READ_FAILED");
      return;
    }

    currentTemp = t;
    currentHum = h;

    if (autoMode) {
      if (currentTemp > targetThreshold + 0.5 && !relayState) {
        setRelay(true);
      } else if (currentTemp < targetThreshold - 0.5 && relayState) {
        setRelay(false);
      }
    }

    // Build documented telemetry packet
    String packet = "TELEM|TEMP:" + String(currentTemp, 1) +
                    "|HUM:" + String(currentHum, 1) +
                    "|THRESH:" + String(targetThreshold, 1) +
                    "|RELAY:" + String(relayState ? "1" : "0") +
                    "|MODE:" + String(autoMode ? "AUTO" : "MANUAL");

    // Send simultaneously to PC USB and MKR1010
    Serial.println(packet);
    mkrSerial.println(packet);
  }
}
