# 🚀 STM32 Distributed CAN System with RTOS Gateway & BLE

A complete distributed embedded system built with STM32 microcontrollers, a 500 kbps CAN bus, and a FreeRTOS-based BLE gateway.

This project demonstrates:

- Multi-node CAN network
- Custom binary protocol over CAN
- RTOS-based gateway architecture
- CAN ↔ BLE bridging
- Fragment reassembly
- Offline supervision
- Android BLE integration
- Deterministic memory management

---

# 🧠 System Architecture

<p align="center">
  <img src="/docs/images/sytem_architecture.png" width="1000"/>
  <br>
  <em>CAN System with RTOS Gateway & BLE</em>
</p>

---

# 🌡 Environment Node

Reads environmental data:

- BMP280 → Temperature, Pressure, Altitude
- AHT20 → Temperature, Humidity

Features:

- Periodic sampling (10s)
- 10-byte sensor packet
- 2-fragment CAN transmission
- Versioned protocol
- LED feedback

Demonstrates:

- I2C drivers
- Sensor abstraction
- CAN protocol fragmentation

---

# 🟢 Motion Node

Reads PIR sensor state.

Features:

- Motion detection
- Heartbeat frames
- CAN transmission
- LED indication
- Command reception (LED ON/OFF)

Demonstrates:

- GPIO interrupt handling
- Lightweight CAN messaging
- Protocol version validation

---

# 🔵 RTOS Gateway (FreeRTOS)

Core of the system.

Runs on STM32F103C6T6 using CMSIS-OS.

Features:

- Multi-task architecture
- CAN RX interrupt → RTOS queue
- Memory pool (no malloc)
- Fragment reassembly
- BLE framed protocol
- Offline detection (15s timeout)
- Command translation (BLE ↔ CAN)

RTOS Tasks:

- CanRxTask
- BleRxTask
- GatewayCANTask
- GatewayBLETask
- SupervisionTask

Demonstrates:

- Deterministic concurrency
- Queue-based communication
- Binary semaphore signaling
- Layered firmware design

---

# 📡 Communication Layers

## CAN Protocol

- 8-byte fixed frames
- Version field
- Message type field
- Counter
- Fragment index
- Payload

Supports:

- Motion telemetry
- Environment telemetry (fragmented)
- Command frames
- Heartbeat

Bitrate: 500 kbps

---

## BLE Protocol

Custom framed transport:

```
[0xAA][0x55][LEN][TYPE][PAYLOAD][CRC16]
```

Features:

- Stateful parser
- CRC16 validation
- Bidirectional communication
- Binary-safe transport

Transported via:

- HM-10 BLE module
- UART 9600 8N1

---

# 🔎 Supervision & Fault Detection

Gateway monitors each node:

```
OFFLINE_TIMEOUT = 15000 ms
```

If timeout occurs:

- LED OFF
- BLE status frame sent
- Node marked offline

Demonstrates distributed system reliability concepts.

---

# 🎛 Command System

Android → BLE → Gateway → CAN → Node

Commands supported:

- All LEDs ON
- All LEDs OFF
- Motion LED control
- Environment LED control

Demonstrates full bidirectional protocol bridge.

---

# ⚙️ Hardware Overview

This distributed system consists of three STM32 nodes interconnected via CAN and a BLE gateway.

---

## 🧠 STM32F103 (Blue Pill)

Used in:
- Motion Node
- Environment Node
- RTOS Gateway

Specifications:
- 72 MHz (HSE 8 MHz + PLL x9)
- CAN1 peripheral
- USART1 (BLE on Gateway)
- I2C1 (Environment Node sensors)
- FreeRTOS (Gateway only)

---

## 🚗 MCP2551 (CAN Transceiver)

Used in all CAN nodes.

- 5V CAN transceiver
- ISO 11898-2 compliant
- Converts MCU CAN TX/RX to CANH/CANL differential bus
- Requires 120Ω termination resistor at both ends of the bus
- High-speed mode (RS pin to GND)

Bitrate configured:
- 500 kbps

---

## 📡 HM-10 (BLE 4.0 Module)

Used in:
- RTOS Gateway

Specifications:
- BLE 4.0
- Transparent UART mode
- 9600 baud
- 3.3V logic
- Connected to USART1

Purpose:
- Wireless bridge between CAN network and Android application

---

## 🟢 HC-SR501 PIR Sensor (Motion Node)

Used in:
- Motion Node

Specifications:
- 5V powered
- Digital output (HIGH on motion)
- Adjustable sensitivity and delay
- Output connected to STM32 GPIO input

Purpose:
- Detect human motion
- Trigger CAN motion frame
- Periodic heartbeat transmission

---

## 🌡 BMP280 (Environment Node)

Interface:
- I2C

Measures:
- Temperature
- Atmospheric pressure
- Altitude (calculated)

Specifications:
- 3.3V logic
- Address: 0x77
- Oversampling configurable
- Internal calibration compensation

Purpose:
- Environmental telemetry over CAN

---

## 💧 AHT20 (Environment Node)

Interface:
- I2C

Measures:
- Temperature
- Relative humidity

Specifications:
- 3.3V logic
- I2C Address: 0x38
- Digital calibrated output

Purpose:
- Complement BMP280 for humidity sensing

---

# 🧩 System Hardware Summary

| Node             | Components                           |
|------------------|--------------------------------------|
| Motion Node      | STM32F103 + MCP2551 + HC-SR501       |
| Environment Node | STM32F103 + MCP2551 + BMP280 + AHT20 |
| Gateway          | STM32F103 + MCP2551 + HM-10          |

---

---

# 🛠 Development Tools

- STM32CubeIDE
- STM32CubeMX
- FreeRTOS (CMSIS-OS)
- Android (Kotlin + Compose)

---

# 🎯 Engineering Highlights

- Real multi-node embedded network
- RTOS-based gateway design
- Deterministic memory usage
- Fragment reassembly logic
- Offline supervision system
- Custom dual-layer protocol
- Clean modular architecture
- ISR → Queue → Task design pattern

---

# 🧪 How to Test the Full System

1. Flash Motion Node
2. Flash Environment Node
3. Flash Gateway
4. Power CAN bus (500 kbps)
5. Connect Android to HM-10
6. Observe:
    - Motion frames
    - Environmental telemetry
    - Online indicators
7. Disconnect a node
8. Observe offline detection after 15s

---

# 📚 What This Project Demonstrates

This repository showcases real-world embedded system design:

- Distributed microcontroller communication
- Protocol design
- Real-time operating systems
- Fault supervision
- Hardware/software integration
- BLE bridging
- Clean firmware architecture

---

## 👤 Author
Javier Rivera  
GitHub: JavierRiv0826