# 🏗 System Architecture

## 1️⃣ Overview

This project implements a distributed embedded system composed of:

- 🟦 Motion CAN Node
- 🌡️ Environment CAN Node
- 🧠 BLE Gateway Node (FreeRTOS)
- 📱 Android Application
- 🐍 Python Debug Tool

Communication is divided into two layers:

- Field Layer → CAN Bus (500 kbps)
- Wireless Layer → BLE 4.0 (HM-10, UART transparent mode)

---

# 2️⃣ System Block Diagram

Motion Node (STM32)
│
│  CAN 500 kbps
│
Environment Node (STM32)
│
▼
BLE Gateway Node (STM32 + FreeRTOS)
│
│ UART (9600 baud)
▼
HM-10 BLE Module
│
▼
Android / Python Host

---

# 3️⃣ Node Responsibilities

## 🟦 Motion Node

- Reads PIR sensor (HC-SR501)
- Sends motion events via CAN
- Sends heartbeat frames
- Executes LED command from Gateway

---

## 🌡️ Environment Node

- Reads:
    - BMP280 (temperature, pressure, altitude)
    - AHT20 (temperature, humidity)
- Sends fragmented CAN telemetry
- Sends heartbeat frames
- Executes LED command from Gateway

---

## 🧠 Gateway Node (FreeRTOS)

Core responsibilities:

- CAN frame reception
- Fragment reassembly (ENV node)
- Offline supervision (15s timeout)
- BLE frame building with CRC16
- BLE command parsing
- Command routing to CAN nodes

This node runs multiple RTOS tasks for deterministic behavior.

---

# 4️⃣ RTOS Task Architecture (Gateway)

| Task            | Purpose              |
|-----------------|----------------------|
| CanRxTask       | Receives CAN frames  |
| GatewayCANTask  | Processes CAN frames |
| BleRxTask       | Receives BLE bytes   |
| GatewayBLETask  | Processes BLE frames |
| SupervisionTask | Offline detection    |
| DefaultTask     | Idle placeholder     |

Inter-task communication:

- Message Queues
- Memory Pool (CAN frames)
- Semaphore (BLE RX)

---

# 5️⃣ Data Flow Direction

## Forward Path (Telemetry)

Sensors → STM32 Node → CAN → Gateway → BLE → Android

## Reverse Path (Command)

Android → BLE → Gateway → CAN → Target Node

---

# 6️⃣ Design Characteristics

- Deterministic CAN frame size (8 bytes)
- Fragmentation for large payloads
- RTOS-based separation of concerns
- CRC-protected BLE framing
- Offline supervision logic
- Modular layered architecture

---

# 7️⃣ Scalability

The system can be extended with:

- Additional CAN nodes
- New message types
- Protocol version updates
- Encrypted BLE transport
- OTA update layer

The architecture separates transport, protocol, and application logic for extensibility.

---