# 🌱 Solar-Based-Smart-Agriculture-System

An IoT-based Smart Agriculture System built with **ESP32**, **FreeRTOS**, and **Blynk IoT** that automatically monitors environmental conditions and controls irrigation while minimizing power consumption through **Deep Sleep Mode** that uses a solar panel and a backup power source to ensure uninterrupted operation in outdoor environments.

---

## 📌 Overview

This project continuously monitors:

* 🌡️ Temperature
* 💧 Humidity
* 🌱 Soil Moisture
* 🚰 Water Tank Level
* 🌧️ Rain Detection
* 🔋 ESP32 Supply Voltage

Based on these sensor readings, the system automatically decides whether irrigation is required and controls a relay-driven water pump accordingly. Sensor data and system status are displayed in real time on the **Blynk IoT dashboard**.

To improve energy efficiency, the ESP32 enters **Deep Sleep Mode** whenever irrigation is unnecessary and wakes up either:

* Every **30 seconds**, or
* Immediately when rainfall is detected via an interrupt.

To ensure reliable operation in outdoor environments, the system employs a hybrid power management architecture powered primarily by a solar panel with a DC adapter serving as a backup power source. An Automatic Transfer Switching (ATS) circuit implemented using 1N5819 Schottky diodes seamlessly switches between the solar panel and the backup supply whenever the solar output becomes insufficient, ensuring uninterrupted operation without requiring software intervention.

---

# ✨ Features

* Real-time environmental monitoring
* Automatic irrigation decision making
* Soil moisture percentage calculation
* Water tank level monitoring
* Rain detection using interrupt
* Automatic relay-controlled water pump
* ESP32 supply voltage monitoring
* Blynk IoT integration
* Deep Sleep power saving
* FreeRTOS dual-core task scheduling
* Wake-up by timer or rain sensor interrupt
* Serial monitoring for debugging
* Hybrid solar and adapter power supply
* Automatic Transfer Switching (ATS)
* Automatic power source selection using 1N5819 Schottky diodes
* Continuous operation during low solar conditions

---

# 🛠 Hardware Components

| Component                           |    Quantity |
| ----------------------------------- | ----------: |
| ESP32 Development Board             |           1 |
| DHT11 Temperature & Humidity Sensor |           1 |
| Capacitive Soil Moisture Sensor     |           1 |
| Water Level Sensor                  |           1 |
| Rain Sensor Module                  |           1 |
| Relay Module                        |           1 |
| Water Pump                          |           1 |
| Voltage Divider (100kΩ + 100kΩ)     | 2 Resistors |
| Solar Panel	                        |           1	|
| DC Power Adapter                    |           1	|
|1N5819 Schottky Diodes               |           2	|

---

# 📚 Libraries Used

* WiFi.h
* BlynkSimpleEsp32.h
* DHT.h
* esp_sleep.h
* FreeRTOS (built into ESP32 Arduino Core)

---

# 🔌 Pin Configuration

| ESP32 Pin | Component              |
| --------- | ---------------------- |
| GPIO 4    | DHT11                  |
| GPIO 34   | Soil Moisture Sensor   |
| GPIO 35   | Water Level Sensor     |
| GPIO 27   | Rain Sensor            |
| GPIO 26   | Relay                  |
| GPIO 32   | Supply Voltage Monitor |

---

# 🧠 System Workflow

1. The system is powered using a **solar panel** as the primary energy source. If the solar output becomes insufficient, the **ATS circuit** automatically switches the power supply to the **backup DC adapter** using **1N5819 Schottky diodes**, ensuring uninterrupted operation.

2. Once powered, the ESP32 initializes the sensors, connects to the Wi-Fi network, and establishes communication with the **Blynk IoT** platform.

3. The ESP32 periodically reads data from the **DHT11**, **soil moisture sensor**, **water level sensor**, and **rain sensor**, while also monitoring its own supply voltage.

4. Raw sensor values are processed and converted into meaningful information, including soil moisture percentage, water level percentage, and environmental status.

5. Based on the sensor data, the system determines whether irrigation is required. The water pump is activated only when:

   * The soil is dry or moderately dry,
   * No rainfall is detected, and
   * The water tank contains sufficient water.

6. The current sensor readings, irrigation status, pump status, system health, and ESP32 supply voltage are transmitted to the **Blynk IoT dashboard** for real-time remote monitoring.

7. If irrigation is required, the ESP32 keeps the relay activated to operate the water pump while continuously monitoring environmental conditions.

8. If irrigation is not required, the relay is turned off, Wi-Fi is disconnected, and the ESP32 enters **Deep Sleep Mode** to minimize power consumption.

9. The ESP32 automatically wakes up after **30 seconds** or immediately when a rainfall event is detected through an external interrupt. After waking, it repeats the monitoring and decision-making process.


---

# ☀️ Hybrid Power Management

The Smart Agriculture System is designed with a **hybrid power supply** to ensure uninterrupted operation in outdoor environments.

### Power Sources

* ☀️ Solar Panel (Primary Power Source)
* 🔌 DC Adapter (Backup Power Source)

Under normal conditions, the ESP32 and all connected peripherals are powered by the solar panel. When the solar panel cannot provide sufficient voltage due to cloudy weather, nighttime, or low battery conditions, the system automatically switches to the backup adapter.

### Automatic Transfer Switching (ATS)

An **Automatic Transfer Switching (ATS)** circuit is implemented using **1N5819 Schottky diodes**.

The ATS circuit:

* Automatically selects the power source with the higher available voltage.
* Requires no software control or manual intervention.
* Prevents reverse current from flowing between the solar supply and the adapter.
* Ensures uninterrupted operation of the irrigation system during power source transitions.
* Uses the low forward voltage drop of the **1N5819 Schottky diode** to improve overall power efficiency.


### Power Architecture

```text
                ☀️ Solar Panel
                     │
                 1N5819 Diode
                     │
                     ├──────────────┐
                     │              │
                ESP32 + Sensors + Relay
                     │              │
                     └──────────────┤
                 1N5819 Diode       │
                     │              │
              🔌 DC Adapter (Backup)
```

The ESP32 always receives power from the source with the higher output voltage. When solar power becomes insufficient, the ATS circuit seamlessly transfers the load to the adapter without interrupting system operation.


# 🌱 Irrigation Logic

The pump turns **ON** only when:

* Soil is **Dry** or **Medium Moisture**
* No rain is detected
* Water tank level is sufficient

The pump remains **OFF** when:

* Soil is already wet
* Rain is detected
* Water level is low

---

# 📊 Sensor Status Classification

## Soil Moisture

| Percentage | Status               |
| ---------- | -------------------- |
| <35%       | Dry Soil             |
| 35–65%     | Medium Soil Moisture |
| >65%       | Wet Soil             |

### Water Tank

| Percentage | Status             |
| ---------- | ------------------ |
| <25%       | Low Water Level    |
| 25–60%     | Medium Water Level |
| >60%       | Enough Water       |

---

# 📱 Blynk Dashboard

| Virtual Pin | Data                  |
| ----------- | --------------------- |
| V0          | Temperature           |
| V1          | Humidity              |
| V2          | Soil Moisture (%)     |
| V3          | Soil Status           |
| V4          | Rain Status           |
| V5          | Water Level (%)       |
| V6          | Water Status          |
| V7          | Overall System Status |
| V8          | Pump Status           |
| V9          | ESP32 Supply Voltage  |

---

# ⚡ Power Saving

To reduce power consumption:

* WiFi disconnects before sleep.
* Relay state is held safely.
* ESP32 enters Deep Sleep.
* Wake-up sources:

  * Timer (30 seconds)
  * Rain sensor interrupt

This significantly reduces energy usage compared to continuously running the microcontroller.

---

# 🧵 FreeRTOS Tasks

The project utilizes both cores of the ESP32.

### Task 1 – Blynk Task

Runs continuously to maintain communication with the Blynk cloud.

### Task 2 – Sensor Task

Responsible for:

* Reading sensors
* Making irrigation decisions
* Pump control
* Deep sleep management

A mutex ensures safe access to shared Blynk resources.

---

# 📈 Example Serial Output

```text
--------- Smart Agriculture System ---------

ESP32 Supply Voltage: 4.95 V
Temperature: 29.5 °C
Humidity: 72 %

Soil Moisture: 31 %
Soil Status: Dry soil

Water Level: 84 %
Water Status: Enough water

Rain Status: No rain

Pump Status: Pump ON

Overall Status:
Irrigation needed. Pump ON.
```

---

# 🎯 Applications

* Smart farming
* Greenhouse automation
* Home gardening
* Precision agriculture
* Water conservation
* IoT-based irrigation systems

---

# 👨‍💻 Developed With

* ESP32
* Arduino IDE
* FreeRTOS
* Blynk IoT Platform

---

# 📄 License

This project is released under the MIT License. Feel free to use, modify, and improve it for educational or research purposes.

---

## ⭐ If you found this project useful

Please consider giving the repository a ⭐ to support future development!
