/*
  Smart Plant Monitoring and Automatic Irrigation System
  Board: ESP32

  Features:
  - Reads soil moisture from an analog soil moisture sensor
  - Converts sensor value to moisture percentage
  - Reads temperature and humidity from DHT11/DHT22
  - Turns water pump on automatically when soil is dry
  - Sends sensor data to Blynk mobile dashboard
*/

#define BLYNK_TEMPLATE_ID "YOUR_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "Smart Plant Irrigation"
#define BLYNK_AUTH_TOKEN "YOUR_BLYNK_AUTH_TOKEN"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

char ssid[] = "YOUR_WIFI_NAME";
char password[] = "YOUR_WIFI_PASSWORD";

const int SOIL_SENSOR_PIN = 34;
const int RELAY_PIN = 26;
const int LED_DRY_PIN = 2;
const int DHT_PIN = 4;

const int DHT_TYPE = DHT11;

const int DRY_VALUE = 4095;
const int WET_VALUE = 1500;
const int DRY_THRESHOLD_PERCENT = 30;
const int WATERING_TIME_MS = 5000;
const int READING_INTERVAL_MS = 2000;

const bool RELAY_ACTIVE_HIGH = true;

DHT dht(DHT_PIN, DHT_TYPE);
BlynkTimer timer;

void setPump(bool isOn) {
  if (RELAY_ACTIVE_HIGH) {
    digitalWrite(RELAY_PIN, isOn ? HIGH : LOW);
  } else {
    digitalWrite(RELAY_PIN, isOn ? LOW : HIGH);
  }
}

int readMoisturePercent() {
  int rawValue = analogRead(SOIL_SENSOR_PIN);
  int moisturePercent = map(rawValue, DRY_VALUE, WET_VALUE, 0, 100);
  moisturePercent = constrain(moisturePercent, 0, 100);
  return moisturePercent;
}

void sendSensorData() {
  int moisturePercent = readMoisturePercent();
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  Serial.print("Soil Moisture: ");
  Serial.print(moisturePercent);
  Serial.println("%");

  if (!isnan(temperature)) {
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" C");
  }

  if (!isnan(humidity)) {
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println("%");
  }

  Blynk.virtualWrite(V0, moisturePercent);
  Blynk.virtualWrite(V1, temperature);
  Blynk.virtualWrite(V2, humidity);

  if (moisturePercent < DRY_THRESHOLD_PERCENT) {
    digitalWrite(LED_DRY_PIN, HIGH);
    Blynk.logEvent("dry_soil", "Soil is dry. Automatic watering started.");

    setPump(true);
    delay(WATERING_TIME_MS);
    setPump(false);
  } else {
    digitalWrite(LED_DRY_PIN, LOW);
    setPump(false);
  }

  Serial.println("--------------------");
}

BLYNK_WRITE(V3) {
  int manualPumpState = param.asInt();
  setPump(manualPumpState == 1);
}

void setup() {
  Serial.begin(115200);

  pinMode(SOIL_SENSOR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_DRY_PIN, OUTPUT);

  setPump(false);
  digitalWrite(LED_DRY_PIN, LOW);

  dht.begin();
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);

  timer.setInterval(READING_INTERVAL_MS, sendSensorData);
}

void loop() {
  Blynk.run();
  timer.run();
}
