# IoT Smart Thermostat Prototype (SOB 13 & SOB 21)

A dual-microcontroller IoT climate-control prototype replicating the closed-loop sensing and actuation behavior of a **Google Nest Learning Thermostat**.

## System Architecture
- **Arduino Uno (5V):** Reads ambient temperature and humidity from a **DHT22** sensor every 2.5 seconds, executes a `±0.5°C` hysteresis control loop, drives an **Active-HIGH 5V relay**, and handles USB Serial communication (`115200 bps`).
- **Arduino MKR WiFi 1010 (3.3V):** Bridges telemetry and control commands between the Uno (`9600 bps` UART via a `1kΩ / 2kΩ` voltage divider) and **Blynk IoT Cloud** (`V0`–`V4`).

## Documented Serial Protocol
- **Telemetry (`Uno → PC & MKR1010`):** `TELEM|TEMP:<float>|HUM:<float>|THRESH:<float>|RELAY:<0|1>|MODE:<AUTO|MANUAL>`
- **Commands (`PC / Blynk → Uno`):** `CMD:THRESH:<float>`, `CMD:RELAY:<0|1>`, `CMD:MODE:<AUTO|MANUAL>`
