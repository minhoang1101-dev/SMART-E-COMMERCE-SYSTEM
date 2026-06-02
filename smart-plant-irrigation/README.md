# Smart Plant Monitoring and Automatic Irrigation System

This is the Arduino source code for the ESP32 project described in the proposal.

## Required Libraries

- Blynk
- DHT sensor library
- Adafruit Unified Sensor

## Pin Connections

| Component | ESP32 Pin |
| --- | --- |
| Soil moisture analog output | GPIO 34 |
| Relay input | GPIO 26 |
| Status LED | GPIO 2 |
| DHT11/DHT22 data pin | GPIO 4 |

## Blynk Virtual Pins

| Virtual Pin | Purpose |
| --- | --- |
| V0 | Soil moisture percentage |
| V1 | Temperature |
| V2 | Humidity |
| V3 | Manual pump switch |

## Setup

1. Open `SmartPlantIrrigation/SmartPlantIrrigation.ino` in Arduino IDE.
2. Install the ESP32 board package.
3. Install the required libraries.
4. Replace these values in the code:
   - `YOUR_TEMPLATE_ID`
   - `YOUR_BLYNK_AUTH_TOKEN`
   - `YOUR_WIFI_NAME`
   - `YOUR_WIFI_PASSWORD`
5. Upload the code to the ESP32.

## Calibration

Adjust these values if your soil moisture sensor gives different readings:

```cpp
const int DRY_VALUE = 4095;
const int WET_VALUE = 1500;
```

Use the Serial Monitor to check real sensor readings in dry soil and wet soil, then update the values.
