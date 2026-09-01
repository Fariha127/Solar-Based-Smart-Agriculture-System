# 🌱 Solar-Based Smart Agriculture System

An IoT-based Smart Agriculture System built with **ESP32**, **FreeRTOS**, and **Blynk IoT** that automatically monitors environmental conditions and controls irrigation while minimizing power consumption through **Deep Sleep Mode**. The system is powered by a **solar panel** with a **DC adapter backup**, ensuring uninterrupted operation in outdoor environments.
---

## 📖 Table of Contents

- [Overview](#-overview)
- [Features](#-features)
- [System Architecture](#-system-architecture)
- [Hardware Components](#-hardware-components)
- [Software Components](#-software-components)
- [Pin Configuration](#-pin-configuration)
- [Circuit Diagram](#-circuit-diagram)
- [Setup Instructions](#-setup-instructions)
- [System Workflow](#-system-workflow)
- [Hybrid Power Management](#️-hybrid-power-management)
- [Irrigation Logic](#-irrigation-logic)
- [Sensor Status Classification](#-sensor-status-classification)
- [Blynk Dashboard](#-blynk-dashboard)
- [Power Saving](#-power-saving)
- [FreeRTOS Tasks](#-freertos-tasks)
- [Example Serial Output](#-example-serial-output)
- [Applications](#-applications)

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

To ensure reliable operation outdoors, the system employs a **hybrid power management architecture** powered primarily by a solar panel, with a DC adapter as backup. An **Automatic Transfer Switching (ATS)** circuit built from **1N5819 Schottky diodes** seamlessly switches between the two sources whenever solar output is insufficient — with no software intervention required.


---

## ✨ Features

* Real-time environmental monitoring
* Automatic irrigation decision making
* Soil moisture percentage calculation
* Water tank level monitoring
* Rain detection using interrupt
* Automatic relay-controlled water pump
* ESP32 supply voltage monitoring
* Blynk IoT integration (live dashboard + remote control)
* Deep Sleep power saving
* FreeRTOS dual-core task scheduling
* Wake-up by timer or rain sensor interrupt
* Serial monitoring for debugging
* Hybrid solar + adapter power supply
* Automatic Transfer Switching (ATS)
* Automatic power source selection using 1N5819 Schottky diodes
* Continuous operation during low solar conditions

---

## 🏗 System Architecture

```text
        ┌────────────┐        ┌────────────────┐
        │ Solar Panel│        │   DC Adapter    │
        └─────┬──────┘        └────────┬────────┘
              │  1N5819               │  1N5819
              │  Diode                │  Diode
              └───────────┬───────────┘
                           │  (ATS: higher voltage wins)
                     ┌─────▼─────┐
                     │   ESP32   │
                     └─────┬─────┘
       ┌───────┬───────────┼──────────────┬─────────┐
       │       │           │              │         │
   ┌───▼──┐┌───▼───┐  ┌────▼────┐   ┌─────▼─── ┐ ┌───▼───┐
   │DHT11 ││ Soil  │  │  Water  │   │  Rain    │ │ Relay │
   │Temp/ ││Moisture│ │  Level  │  | Sensor   │ │ +Pump │
   │Humid ││Sensor  │ │ Sensor  │  |(Interrupt)│|       │
   └──────┘└────────┘ └─────────┘  |─────────┘  └───────┘
                           │
                     ┌─────▼──────┐
                     │  Wi-Fi /   │
                     │ Blynk IoT  │
                     │ Dashboard  │
                     └────────────┘
```

**Data flow:** Sensors → ESP32 (FreeRTOS tasks) → Irrigation decision logic → Relay/Pump control + Blynk dashboard update → Deep Sleep (timer or rain-interrupt wake).

---

## 🛠 Hardware Components

| Component                           |    Quantity |
| ------------------------------------ | ----------: |
| ESP32 Development Board              |           1 |
| DHT11 Temperature & Humidity Sensor  |           1 |
| Capacitive Soil Moisture Sensor      |           1 |
| Water Level Sensor                   |           1 |
| Rain Sensor Module                   |           1 |
| Relay Module                         |           1 |
| Water Pump                           |           1 |
| Voltage Divider (100kΩ + 100kΩ)      | 2 Resistors |
| Solar Panel                          |           1 |
| DC Power Adapter                     |           1 |
| 1N5819 Schottky Diodes               |           2 |

---

## 📚 Software Components

| Library / Tool                        | Purpose                              |
| -------------------------------------- | ------------------------------------- |
| [Arduino IDE](https://www.arduino.cc/en/software) | Firmware development & upload |
| `WiFi.h`                               | ESP32 Wi-Fi connectivity              |
| `BlynkSimpleEsp32.h`                   | Blynk IoT cloud communication         |
| `DHT.h`                                | DHT11 temperature/humidity readings   |
| `esp_sleep.h`                          | Deep Sleep & wake-source configuration|
| FreeRTOS (built into ESP32 Arduino Core) | Dual-core task scheduling           |
| [Blynk IoT App/Console](https://blynk.io/) | Remote dashboard & monitoring     |

---

## 🔌 Pin Configuration

| ESP32 Pin | Component              |
| --------- | ----------------------- |
| GPIO 4    | DHT11                   |
| GPIO 34   | Soil Moisture Sensor    |
| GPIO 35   | Water Level Sensor      |
| GPIO 27   | Rain Sensor             |
| GPIO 26   | Relay                   |
| GPIO 32   | Supply Voltage Monitor  |

---

## ⚡ Circuit Diagram

Refer to the [Hybrid Power Management](#️-hybrid-power-management) section for the ATS wiring between the solar panel, DC adapter, and ESP32.

---

## 🚀 Setup Instructions

### 1. Hardware Assembly
1. Wire the DHT11, soil moisture sensor, water level sensor, and rain sensor to the GPIOs listed in [Pin Configuration](#-pin-configuration).
2. Connect the relay module's signal pin to GPIO 26, with the water pump wired through the relay's switched output.
3. Build the voltage divider (100kΩ + 100kΩ) from the ESP32 supply rail to GPIO 32 to safely monitor supply voltage.
4. Wire the solar panel and DC adapter to the ESP32 power input, each through a 1N5819 Schottky diode, forming the ATS circuit as shown in the [architecture diagram](#-system-architecture).

### 2. Software Setup
1. Install the [Arduino IDE](https://www.arduino.cc/en/software) (1.8.x or 2.x).
2. Add ESP32 board support via **File → Preferences → Additional Board Manager URLs**:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Install the ESP32 board package via **Tools → Board → Boards Manager**.
4. Install required libraries via **Sketch → Include Library → Manage Libraries**:
   - `Blynk` (Blynk IoT library)
   - `DHT sensor library` (Adafruit)
5. Clone this repository:
   ```bash
   git clone https://github.com/<your-username>/Solar-Based-Smart-Agriculture-System.git
   ```

### 3. Blynk Configuration
1. Create a project on the [Blynk Console](https://blynk.cloud/).
2. Add datastreams for virtual pins V0–V9 as listed in the [Blynk Dashboard](#-blynk-dashboard) table.
3. Copy your **Blynk Auth Token** and **Template ID/Name**.
4. In the sketch, set your credentials:
   ```cpp
   #define BLYNK_TEMPLATE_ID   "your_template_id"
   #define BLYNK_TEMPLATE_NAME "your_template_name"
   #define BLYNK_AUTH_TOKEN    "your_auth_token"

   char ssid[] = "your_wifi_ssid";
   char pass[] = "your_wifi_password";
   ```

### 4. Flash & Run
1. Select **Tools → Board → ESP32 Dev Module** and the correct COM port.
2. Upload the sketch.
3. Open the Serial Monitor (115200 baud) to verify sensor readings and Wi-Fi/Blynk connection.
4. Open the Blynk app/console to confirm live data and control the pump remotely.

---

## 🧠 System Workflow

1. The system is powered using a **solar panel** as the primary energy source. If solar output becomes insufficient, the **ATS circuit** automatically switches to the **backup DC adapter** using **1N5819 Schottky diodes**, ensuring uninterrupted operation.
2. Once powered, the ESP32 initializes sensors, connects to Wi-Fi, and establishes communication with **Blynk IoT**.
3. The ESP32 periodically reads data from the **DHT11**, **soil moisture sensor**, **water level sensor**, and **rain sensor**, while monitoring its own supply voltage.
4. Raw sensor values are converted into meaningful information: soil moisture %, water level %, and environmental status.
5. Based on sensor data, the system determines whether irrigation is required. The pump activates only when:
   * The soil is dry or moderately dry,
   * No rainfall is detected, and
   * The water tank contains sufficient water.
6. Sensor readings, irrigation status, pump status, system health, and supply voltage are sent to the **Blynk IoT dashboard**.
7. If irrigation is required, the relay stays active while the system keeps monitoring conditions.
8. If irrigation is not required, the relay turns off, Wi-Fi disconnects, and the ESP32 enters **Deep Sleep Mode**.
9. The ESP32 wakes up after **30 seconds** or immediately on a rainfall interrupt, then repeats the cycle.

---

## ☀️ Hybrid Power Management

The system uses a **hybrid power supply** for uninterrupted outdoor operation.

**Power Sources**
* ☀️ Solar Panel (Primary)
* 🔌 DC Adapter (Backup)

Under normal conditions, the ESP32 and peripherals run on solar power. When solar output drops too low (cloudy weather, nighttime, low battery), the system automatically switches to the backup adapter.

**Automatic Transfer Switching (ATS)**

Implemented using **1N5819 Schottky diodes**, the ATS circuit:
* Automatically selects the source with higher available voltage.
* Requires no software control or manual intervention.
* Prevents reverse current between the solar supply and the adapter.
* Ensures uninterrupted operation during power source transitions.
* Uses the diode's low forward-voltage drop to improve efficiency.

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

---

## 🌱 Irrigation Logic

**Pump ON** when:
* Soil is **Dry** or **Medium Moisture**
* No rain is detected
* Water tank level is sufficient

**Pump OFF** when:
* Soil is already wet
* Rain is detected
* Water level is low

---

## 📊 Sensor Status Classification

**Soil Moisture**

| Percentage | Status               |
| ---------- | --------------------- |
| <35%       | Dry Soil               |
| 35–65%     | Medium Soil Moisture   |
| >65%       | Wet Soil                |

**Water Tank**

| Percentage | Status              |
| ---------- | --------------------- |
| <25%       | Low Water Level        |
| 25–60%     | Medium Water Level     |
| >60%       | Enough Water            |

---

## 📱 Blynk Dashboard

| Virtual Pin | Data                   |
| ----------- | ----------------------- |
| V0          | Temperature              |
| V1          | Humidity                 |
| V2          | Soil Moisture (%)        |
| V3          | Soil Status               |
| V4          | Rain Status                |
| V5          | Water Level (%)             |
| V6          | Water Status                 |
| V7          | Overall System Status         |
| V8          | Pump Status                     |
| V9          | ESP32 Supply Voltage              |

> 📷 *Add a Blynk dashboard screenshot here, e.g. `![Blynk Dashboard](images/blynk-dashboard.png)`*

---

## ⚡ Power Saving

* Wi-Fi disconnects before sleep.
* Relay state is held safely.
* ESP32 enters Deep Sleep.
* Wake-up sources:
  * Timer (30 seconds)
  * Rain sensor interrupt

This significantly reduces energy usage compared to continuously running the microcontroller.

---

## 🧵 FreeRTOS Tasks

The project uses both cores of the ESP32.

**Task 1 – Blynk Task**
Runs continuously to maintain communication with the Blynk cloud.

**Task 2 – Sensor Task**
Responsible for:
* Reading sensors
* Making irrigation decisions
* Pump control
* Deep sleep management

A mutex ensures safe access to shared Blynk resources between tasks.

---

## 📈 Example Serial Output

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

## 🎯 Applications

* Smart farming
* Greenhouse automation
* Home gardening
* Precision agriculture
* Water conservation
* IoT-based irrigation systems

---

## 👨‍💻 Developed With

* ESP32
* Arduino IDE
* FreeRTOS
* Blynk IoT Platform

---
