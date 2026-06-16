# Sentinel — ESP32 Air Quality Monitor Firmware

ESP-IDF firmware for an indoor air quality monitoring node built on the ESP32 WROVER. Reads temperature, humidity, eCO2, and TVOC from a DHT22 and SGP30 sensor, processes the data through a FreeRTOS task pipeline, and publishes JSON readings to an MQTT broker over TLS WiFi.

## Architecture

Three concurrent FreeRTOS tasks:

- **sensor_task** — reads DHT22 and SGP30 every second, pushes to a FreeRTOS queue
- **processing_task** — applies EMA smoothing filter, evaluates alert thresholds, pushes processed readings to a second queue  
- **mqtt_task** — connects to WiFi, establishes TLS MQTT connection, publishes JSON payload every second

## Hardware

| Component | Pin |
|-----------|-----|
| DHT22 Data | GPIO4 |
| SGP30 SDA | GPIO21 |
| SGP30 SCL | GPIO22 |

## Sensors

- **DHT22** — temperature and humidity. Custom bit-bang GPIO driver, no libraries
    Note: bare DHT22 sensors have 4 pins (VCC, Data, NC, GND) and require an external 
    10kΩ pull-up resistor between the data line and 3.3V. Module versions have 3 pins 
    (VCC, Data, GND) with the resistor built in — no external resistor needed.
- **SGP30** — eCO2 (ppm) and TVOC (ppb). Custom I2C driver with CRC verification and humidity compensation

## Setup

1. Install ESP-IDF v6.0
2. Copy `secrets.h.example` to `secrets.h` and fill in your credentials
3. `idf.py set-target esp32`
4. `idf.py build`
5. `idf.py -p /dev/ttyUSB0 flash monitor`

## MQTT Payload

```json
{
  "temp": 24.31,
  "humidity": 47.20,
  "eco2": 842,
  "tvoc": 12,
  "alerts": {
    "temp_high": false,
    "temp_low": false,
    "humidity_high": false,
    "humidity_low": false,
    "eco2_warn": false,
    "eco2_bad": false,
    "tvoc_warn": false,
    "tvoc_bad": false
  },
  "ts": 1746000000000
}
```

## Dependencies

- ESP-IDF v6.0
- FreeRTOS (bundled)
- esp-mqtt (bundled)
- mbedTLS (bundled)
