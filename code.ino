#define BLYNK_TEMPLATE_ID "TMPL6duA9DNYp"
#define BLYNK_TEMPLATE_NAME "Smart Agriculture System"
#define BLYNK_AUTH_TOKEN "pIwX2tUuOTmeqf1m7VR7nAmQldYBNQH1"
#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include "esp_sleep.h"
#include "driver/rtc_io.h"
#include "driver/gpio.h"
#include "soc/soc.h"             
#include "soc/rtc_cntl_reg.h"    
// WiFi information 
// char ssid[] = "Suhita";
// char pass[] = "q9mnwpgj";
char ssid[] = "Student's LAB";
char pass[] = "kuet@cse321";
//  char ssid[] = "TP-Link_FCE2";
// char pass[] = "Succulent@plant";
//  Sleep settings 
#define SLEEP_TIME_SECONDS 30
// Sensor pins 
#define SUPPLY_VOLTAGE_PIN 32
#define DHTPIN 4
#define DHTTYPE DHT11
#define SOIL_PIN 34
#define WATER_PIN 35
#define RAIN_PIN 27
// Relay pin
#define RELAY_PIN 26
DHT dht(DHTPIN, DHTTYPE);
// RTOS task handles
TaskHandle_t blynkTaskHandle = NULL;
TaskHandle_t sensorTaskHandle = NULL;
SemaphoreHandle_t blynkMutex;
// Interrupt flag
volatile bool rainInterruptFlag = false;
// Calibration values 
int SOIL_DRY_RAW = 3200;
int SOIL_WET_RAW = 1300;
int WATER_EMPTY_RAW = 0;
int WATER_FULL_RAW = 450;
// Thresholds
float HIGH_TEMP_LIMIT = 35.0;
int SOIL_DRY_LIMIT = 35;
int SOIL_WET_LIMIT = 65;
int WATER_LOW_LIMIT = 25;
// Sensor variables 
float esp32SupplyVoltage = 0;
float temperature = 0;
float humidity = 0;
int soilRaw = 0;
int soilPercent = 0;
int waterRaw = 0;
int waterPercent = 0;
bool rainDetected = false;
bool pumpState = false;
//  Rain interrupt 
void IRAM_ATTR rainISR() {
  rainInterruptFlag = true;
}
// Keep percentage 0-100
int limitPercent(int value) {
  if (value < 0) return 0;
  if (value > 100) return 100;
  return value;
}
// Soil percentage 
int getSoilPercent(int rawValue) {
  int percent = map(rawValue, SOIL_DRY_RAW, SOIL_WET_RAW, 0, 100);
  return limitPercent(percent);
}
// Water percentage
int getWaterPercent(int rawValue) {
  int percent = map(rawValue, WATER_EMPTY_RAW, WATER_FULL_RAW, 0, 100);
  return limitPercent(percent);
}
// Soil status 
String getSoilStatus(int percent) {
  if (percent < SOIL_DRY_LIMIT) {
    return "Dry soil";
  }
  else if (percent >= SOIL_DRY_LIMIT && percent <= SOIL_WET_LIMIT) {
    return "Medium soil moisture";
  }
  else {
    return "Wet soil";
  }
}
// Water status
String getWaterStatus(int percent) {
  if (percent < WATER_LOW_LIMIT) {
    return "Low water level";
  }
  else if (percent >= WATER_LOW_LIMIT && percent <= 60) {
    return "Medium water level";
  }
  else {
    return "Enough water";
  }
}
//  Pump control 

// Pump ON  = GPIO LOW
// Pump OFF = GPIO INPUT_PULLUP, like disconnected but safer
void setPump(bool state) {
  pumpState = state;
  if (state) {
    digitalWrite(RELAY_PIN, LOW);
    pinMode(RELAY_PIN, OUTPUT);
  } 
  else {
    pinMode(RELAY_PIN, INPUT_PULLUP);
  }
}
// Pump decision 
bool isPumpNeeded(String soilStatus, String rainStatus, String waterStatus) {
  if ((soilStatus == "Dry soil" || soilStatus == "Medium soil moisture") &&
      rainStatus == "No rain" &&
      (waterStatus == "Enough water" || waterStatus == "Medium water level")) {
    return true;
  } 
  else {
    return false;
  }
}
//  Overall status
String getOverallStatus(String soilStatus, String rainStatus, String waterStatus, float temperature) {
  String status = "";
  if (temperature > HIGH_TEMP_LIMIT) {
    status += "High temperature. ";
  }
  if (waterStatus == "Low water level") {
    status += "Water source low. Pump OFF. ";
  }
  if (rainStatus == "Rain detected") {
    status += "Rain detected. Irrigation not needed. ";
  }
  if ((soilStatus == "Dry soil" || soilStatus == "Medium soil moisture") &&
    rainStatus == "No rain" &&
    waterStatus == "Enough water") {
  status += "Irrigation needed. Pump ON. ";
}
  if (soilStatus == "Wet soil") {
    status += "Soil wet. Pump OFF. ";
  }
  if (status == "") {
    status = "System normal";
  }
  return status;
}
// Read sensors, send data, control pump 
float readESP32SupplyVoltage() {
  int raw = analogRead(SUPPLY_VOLTAGE_PIN);
  // ESP32 ADC range: 0 to 4095, voltage at ADC pin: 0 to 3.3V
  float adcVoltage = (raw / 4095.0) * 3.3;
  // Voltage divider: R1 = 100k, R2 = 100k
  // Actual supply voltage = ADC voltage × 2
  float supplyVoltage = adcVoltage * 2.0;
  return supplyVoltage;
}
bool readSensorsAndControlPump() {
  float tempRead = dht.readTemperature();
  float humRead = dht.readHumidity();
  if (!isnan(tempRead)) {
    temperature = tempRead;
  }
  if (!isnan(humRead)) {
    humidity = humRead;
  }
  soilRaw = analogRead(SOIL_PIN);
  soilPercent = getSoilPercent(soilRaw);
  waterRaw = analogRead(WATER_PIN);
  waterPercent = getWaterPercent(waterRaw);
  esp32SupplyVoltage = readESP32SupplyVoltage();
  int rainValue = digitalRead(RAIN_PIN);
  if (rainValue == LOW) {
    rainDetected = true;
  } 
  else {
    rainDetected = false;
  }
  String rainStatus = rainDetected ? "Rain detected" : "No rain";
  String soilStatus = getSoilStatus(soilPercent);
  String waterStatus = getWaterStatus(waterPercent);
  bool pumpNeeded = isPumpNeeded(soilStatus, rainStatus, waterStatus);
  if (pumpNeeded) {
    setPump(true);
  } 
  else {
    setPump(false);
  }
  String overallStatus = getOverallStatus(soilStatus, rainStatus, waterStatus, temperature);
  String pumpStatus = pumpState ? "Pump ON" : "Pump OFF";
  if (Blynk.connected()) {
    if (xSemaphoreTake(blynkMutex, 100 / portTICK_PERIOD_MS)) {
      Blynk.virtualWrite(V0, temperature);
      Blynk.virtualWrite(V1, humidity);
      Blynk.virtualWrite(V2, soilPercent);
      Blynk.virtualWrite(V3, soilStatus);
      Blynk.virtualWrite(V4, rainStatus);
      Blynk.virtualWrite(V5, waterPercent);
      Blynk.virtualWrite(V6, waterStatus);
      Blynk.virtualWrite(V7, overallStatus);
      Blynk.virtualWrite(V8, pumpStatus);
      Blynk.virtualWrite(V9, esp32SupplyVoltage);
      xSemaphoreGive(blynkMutex);
    }
  }
  Serial.println("--------- Smart Agriculture System ---------");
  Serial.print("ESP32 Supply Voltage: ");
  Serial.print(esp32SupplyVoltage);
  Serial.println(" V");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");
  Serial.print("Soil Raw Value: ");
  Serial.println(soilRaw);
  Serial.print("Soil Moisture: ");
  Serial.print(soilPercent);
  Serial.println(" %");
  Serial.print("Soil Status: ");
  Serial.println(soilStatus);
  Serial.print("Water Raw Value: ");
  Serial.println(waterRaw);
  Serial.print("Water Level: ");
  Serial.print(waterPercent);
  Serial.println(" %");
  Serial.print("Water Status: ");
  Serial.println(waterStatus);
  Serial.print("Rain Status: ");
  Serial.println(rainStatus);
  Serial.print("Pump Status: ");
  Serial.println(pumpStatus);
  Serial.print("Overall Status: ");
  Serial.println(overallStatus);
  if (pumpNeeded) {
    Serial.println("Decision: Pump needed. ESP32 will stay awake.");
  } 
  else {
    Serial.println("Decision: Pump not needed. ESP32 will go to deep sleep.");
  }
  Serial.println("--------------------------------------------");
  Serial.println();
  return pumpNeeded;
}
// Wakeup reason 
void printWakeupReason() {
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) {
    Serial.println("Wakeup reason: Timer wakeup after 30 seconds");
  }
  else if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0) {
    Serial.println("Wakeup reason: Rain sensor interrupt wakeup");
  }
  else {
    Serial.println("Wakeup reason: Normal power ON / reset");
  }
}
//  Deep sleep
void goToDeepSleep() {
  Serial.println("Preparing for deep sleep...");
  // Pump OFF before sleeping because pump is not needed
  setPump(false);
  delay(500);
  // Hold relay OFF state during sleep
  gpio_hold_en((gpio_num_t)RELAY_PIN);
  gpio_deep_sleep_hold_en();
  if (Blynk.connected()) {
    if (xSemaphoreTake(blynkMutex, 100 / portTICK_PERIOD_MS)) {
      Blynk.disconnect();
      xSemaphoreGive(blynkMutex);
    }
  }
  WiFi.disconnect(true);
  delay(500);
  // Wake every 30 seconds
  esp_sleep_enable_timer_wakeup(SLEEP_TIME_SECONDS * 1000000ULL);
  // Wake immediately if rain sensor goes LOW
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_27, 0);
  // Keep rain sensor pin pulled up during sleep
  rtc_gpio_pullup_en(GPIO_NUM_27);
  rtc_gpio_pulldown_dis(GPIO_NUM_27);
  Serial.print("ESP32 sleeping for ");
  Serial.print(SLEEP_TIME_SECONDS);
  Serial.println(" seconds...");
  Serial.println();
  delay(1000);
  esp_deep_sleep_start();
}
// RTOS Task 1: Blynk communication 
void blynkTask(void * parameter) {
  while (true) {
    if (xSemaphoreTake(blynkMutex, 10 / portTICK_PERIOD_MS)) {
      Blynk.run();
      xSemaphoreGive(blynkMutex);
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}
//  RTOS Task 2: Sensor + pump + sleep logic 
void sensorTask(void * parameter) {
  while (true) {
    if (rainInterruptFlag) {
      rainInterruptFlag = false;
      Serial.println("Rain interrupt detected while awake!");
    }
    bool pumpNeeded = readSensorsAndControlPump();
    if (pumpNeeded) {
      // Pump is needed, so ESP32 stays awake
      vTaskDelay(2000 / portTICK_PERIOD_MS);
    } 
    else {
      
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      goToDeepSleep();
    }
  }
}
// Setup 
void setup() {

  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  Serial.begin(115200);
  delay(1000);
  // Release GPIO hold after waking up
  gpio_deep_sleep_hold_dis();
  gpio_hold_dis((gpio_num_t)RELAY_PIN);
  Serial.println();
  Serial.println("Smart Agriculture System Starting...");
  printWakeupReason();
  dht.begin();
  pinMode(RAIN_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RAIN_PIN), rainISR, FALLING);
  // Pump OFF safely at start
  setPump(false);
  analogReadResolution(12);
  pinMode(SUPPLY_VOLTAGE_PIN, INPUT);
  analogSetPinAttenuation(SUPPLY_VOLTAGE_PIN, ADC_11db);
  blynkMutex = xSemaphoreCreateMutex();
  Serial.println("Connecting to Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("Connected to Blynk");
  Serial.println("Smart Agriculture System Started");
  xTaskCreatePinnedToCore(
    blynkTask,
    "Blynk Task",
    10000,
    NULL,
    1,
    &blynkTaskHandle,
    0
  );
  xTaskCreatePinnedToCore(
    sensorTask,
    "Sensor Task",
    10000,
    NULL,
    1,
    &sensorTaskHandle,
    1
  );
}
// Loop 
void loop() {
  // Empty because RTOS tasks are running
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}

