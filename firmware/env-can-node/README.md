# 🌡 STM32 CAN Environment Node

📌 Description  
A modular embedded node built on the STM32F103C6T6 (Blue Pill) responsible for environmental telemetry acquisition and transmission over a 500 kbps CAN bus.

This node collects temperature, humidity, pressure, and altitude data from I2C sensors and periodically transmits them to a BLE gateway using a custom CAN protocol.

The firmware follows a layered architecture:
* Sensor drivers (BMP280 + AHT20 over I2C)
* CAN driver abstraction
* Application layer (packet building, fragmentation, command handling)

This project demonstrates multi-sensor integration, I2C driver development, CAN frame fragmentation, and distributed embedded system design.

<p align="center">
  <img src="/docs/images/env-can-node.jpg" width="1000"/>
  <br>
  <em>STM32 Blue Pill Environmental Node with CAN Bus Interface</em>
</p>

---

## 🚀 Features

### 🌡 Environmental Sensing
* ✔ BMP280 (Temperature, Pressure, Altitude)
* ✔ AHT20 (Temperature, Humidity)
* ✔ I2C @ 100 kHz
* ✔ Fixed-point scaling (×100, ×10 for precision without floats)

### 🚗 CAN Communication (500 kbps)
* ✔ Standard 11-bit CAN identifiers
* ✔ Fragmented transmission (10-byte payload → 2 CAN frames)
* ✔ Rolling frame counter
* ✔ Periodic transmission (10 seconds)
* ✔ Command reception (LED control)

### 🧠 CAN Bus Physical Layer

* Standard: ISO 11898-2 (High-Speed CAN)
* Bitrate: 500 kbps
* Differential signaling (CANH/CANL)
* 120Ω termination at both ends
* Multi-node bus topology

### 🧠 Application Layer
* ✔ Periodic telemetry scheduling
* ✔ CAN RX command parsing
* ✔ GPIO feedback
* ✔ Non-blocking main loop

---

# ⚙️ STM32CubeMX Configuration

## 🔄 Clock
* HSE = 8 MHz
* PLL ×9 → SYSCLK = 72 MHz
* AHB = 72 MHz
* APB1 = 36 MHz
* APB2 = 72 MHz

---

## 🚗 CAN1 Configuration
* Mode: Normal
* Prescaler = 4
* TimeSeg1 = 13 TQ
* TimeSeg2 = 4 TQ
* SJW = 1 TQ
* Auto Bus-Off = Enabled
* Auto Wake-Up = Enabled
* Auto Retransmission = Enabled

### CAN IDs
| Node             | Standard ID |
|------------------|-------------|
| Motion Node      | 0x100       |
| Environment Node | 0x200       |
| Gateway          | 0x300       |

---

## 🔁 I2C1 Configuration
* Clock Speed: 100 kHz
* Addressing Mode: 7-bit
* Duty Cycle: 2
* No Stretch Mode: Disabled

---

## ↔️ GPIO Configuration
* PC13 → Status LED (telemetry indicator)
* PB3 → Command-controlled LED

---

## 🔌 Wiring (AHT20 → STM32)

| AHT20 | STM32F103 |
|-------|-----------|
| VCC   | 3.3V      |
| GND   | GND       |
| SDA   | PB7       |
| SCL   | PB6       |

I2C Address: 0x38

---

## 🔌 Wiring (BMP280 → STM32)

| BMP280 | STM32F103 |
|--------|-----------|
| VCC    | 3.3V      |
| GND    | GND       |
| SDA    | PB7       |
| SCL    | PB6       |

I2C Address: 0x77

Both sensors share the same I2C bus.

---

## 🔌 Wiring (MCP2551 CAN Transceiver → STM32)

| MCP2551 | STM32F103             |
|---------|-----------------------|
| VDD     | 5V                    |
| GND     | GND                   |
| TXD     | PA12 (CAN_TX)         |
| RXD     | PA11 (CAN_RX)         |
| CANH    | CAN Bus High          |
| CANL    | CAN Bus Low           |
| RS      | GND (High-Speed Mode) |

⚠️ 120Ω termination resistor required at both ends of CAN bus.

---

## 📦 Telemetry Packet Structure

Environmental data is packed into a 10-byte sensor packet:

| Field         | Type   | Description         |
|---------------|--------|---------------------|
| bmp_temp_x100 | int16  | Temperature °C ×100 |
| aht_temp_x100 | int16  | Temperature °C ×100 |
| humidity_x100 | uint16 | %RH ×100            |
| pressure_dpa  | uint16 | Pressure (Pa / 10)  |
| altitude_x10  | int16  | Altitude ×10        |

Because CAN payload is limited to 8 bytes, the packet is fragmented into two frames:

### Fragmented CAN Transmission

* Each CAN frame carries 5 bytes of payload
* `fragment_idx = 0` → first half
* `fragment_idx = 1` → second half
* Both frames share the same rolling counter

This ensures reliable reconstruction on the gateway side.

---

## 🔐 Periodic Telemetry

* Transmission period: 10 seconds (`ENV_PERIOD_MS = 10000`)
* Uses `HAL_GetTick()` for scheduling
* LED toggles on successful transmission

---

## 🧩 Drivers Overview

### app.c / app.h
* Periodic sensor reading
* Packet assembly
* Fragmented CAN transmission
* Command processing

### can_driver.c / can_driver.h
* CAN abstraction layer
* TX/RX handling

### bmp280.c / bmp280.h
* Calibration reading
* Raw ADC acquisition
* Fixed-point compensation
* Altitude calculation

### aht20.c / aht20.h
* Measurement triggering
* Raw data parsing
* Temperature & humidity scaling

### protocol.h
* Unified 8-byte CAN frame structure
* Message type definitions
* Fragment index handling

---

## 🏗 Firmware Architecture

```text
+----------------------------+
| Application Layer          |
| - Periodic scheduler       |
| - Packet builder           |
| - Fragmentation logic      |
| - Command handling         |
+----------------------------+
             ↓
+----------------------------+
| Sensor Drivers (I2C)       |
| - BMP280                   |
| - AHT20                    |
+----------------------------+
             ↓
+----------------------------+
| CAN Driver                 |
| - Frame transmit           |
| - Frame receive            |
+----------------------------+
             ↓
+----------------------------+
| STM32 HAL + Hardware       |
| - I2C1                     |
| - CAN1                     |
| - GPIO                     |
+----------------------------+
```

---

## 🗂 Project Structure

```bash
project/
├── Core/
│   └── Src/
│       └── main.c
│
├── CAN/
│   ├── can_driver.c
│   └── can_driver.h
│
├── app/
│   ├── app.c
│   └── app.h
│
├── protocol/
│   └── protocol.h
│
└── sensors/
    ├── aht20/
    │   ├── aht20.c
    │   └── aht20.h
    │
    └── bmp280/
        ├── bmp280.c
        └── bmp280.h
```

---

## 💡 How to Run 🧪

### 1. Clone the Repository
```bash
git clone https://github.com/JavierRiv0826/STM32-RTOS-COM-CAN-BLE4.git
```

### 2. Open in STM32CubeIDE
* File → Open Projects from File System
* Select project folder

### 3. Build Project
Press Ctrl + B

### 4. Flash MCU
* Connect ST-Link
* Press Run or Debug

### 5. Connect to CAN bus (500 kbps)
### 6. Expected Behavior

Every 10 seconds:

- Reads:
    - BMP280 (Temperature, Pressure, Altitude)
    - AHT20 (Temperature, Humidity)
- Builds 10-byte sensor packet
- Sends 2 CAN fragments (5 bytes each)
- Toggles PC13 LED on transmission

---

## 🧠 Engineering Highlights

* Multi-sensor I2C integration
* Fixed-point math (no float dependency in protocol)
* CAN payload fragmentation strategy
* Rolling counter for traceability
* Distributed CAN architecture
* Clean driver-layer separation

---

## 🛠 Development Tools
* STM32CubeIDE
* STM32CubeMX

---

## 👤 Author
Javier Rivera  
GitHub: JavierRiv0826