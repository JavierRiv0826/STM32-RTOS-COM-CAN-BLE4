# 🚀 STM32 CAN + BLE Gateway (RTOS) – Distributed Embedded System

A professional multi-node embedded system built with **STM32 + CAN Bus + BLE + FreeRTOS + Android**.

This project demonstrates:

- 🧠 Real-time embedded firmware design
- 🚌 CAN bus multi-node communication
- 📡 BLE gateway architecture
- 🧩 Custom binary protocol with CRC16 validation
- 📱 Android mobile client (Jetpack Compose)
- 🐍 Python desktop monitoring tool
- 🏗 Clean software architecture (Embedded + Android)

---

# 🏗 System Overview

This is a distributed embedded system composed of:

- 🔵 Motion Node (STM32 + PIR)
- 🌡 Environment Node (STM32 + BMP280 + AHT20)
- 📡 BLE Gateway Node (STM32 + FreeRTOS + CAN + HM-10)
- 📱 Android App
- 🐍 Python BLE Tool

The Gateway bridges the CAN network to BLE clients.

---

# 🧭 System Architecture

<p align="center">
  <img src="/docs/images/sytem_architecture.png" width="1000"/>
  <br>
  <em>CAN System with RTOS Gateway & BLE</em>
</p>

- CAN used for deterministic node communication
- BLE used for wireless monitoring/control
- FreeRTOS used in gateway for task separation

---

# ⚙️ Hardware Stack

## 🔹 Motion Node
- STM32F103 (Blue Pill)
- HC-SR501 PIR sensor
- MCP2551 CAN transceiver

## 🔹 Environment Node
- STM32F103
- BMP280 (Pressure + Temperature)
- AHT20 (Humidity + Temperature)
- MCP2551 CAN transceiver

## 🔹 BLE Gateway Node
- STM32F103
- MCP2551
- HM-10 BLE 4.0 module
- FreeRTOS

---

# 🧠 Firmware Architecture

## Motion Node
- Interrupt-based motion detection
- CAN event transmission
- Periodic heartbeat

## Environment Node
- I2C sensor acquisition
- Fixed-point scaling
- Periodic CAN broadcast
- Heartbeat monitoring

## BLE Gateway (FreeRTOS)

Tasks:
- CAN RX Task
- CAN TX Task
- BLE RX Task
- BLE TX Task
- Frame Parser Task
- Heartbeat Monitor Task

The gateway:
- Parses CAN frames
- Builds BLE frames
- Validates CRC
- Detects node disconnections
- Forwards commands to CAN

---

# 📦 CAN Protocol

All CAN frames use:

- Standard ID (11-bit)
- Fixed 8-byte payload
- Deterministic frame size

Bitrate: **500 kbps**

## CAN Frame Layout (8 Bytes)

| Byte | Description                              |
|------|------------------------------------------|
| 0    | Version (2 bits) + Message Type (6 bits) |
| 1    | Frame Counter                            |
| 2    | Fragment Index                           |
| 3–7  | Payload (5 bytes)                        |


# 📦 Custom Binary BLE Protocol

Framed protocol:

```
[ 0xAA | 0x55 | LEN | TYPE | PAYLOAD | CRC_L | CRC_H ]
```

- CRC16-CCITT
- Stream-safe state machine
- Shared across:
    - STM32 firmware
    - Android app
    - Python tool

Frame types:
- 0x01 → Motion
- 0x02 → Environment
- 0x03 → Status

See:
```
protocol/README.md
```

---

# 📱 Android Application

- BLE scanning
- GATT connection handling
- Real-time telemetry display
- Node disconnection detection
- LED command transmission
- Clean MVVM + Hilt architecture

See:
```
android/README.md
```

---

# 🐍 Python BLE Tool

- BLE scanner
- Binary protocol parser
- CRC validation
- Keyboard-based command control

See:
```
python/README.md
```

---

# 📂 Repository Structure

```
STM32-RTOS-COM-CAN-BLE4/
│
├── README.md
├── LICENSE
├── .gitignore
│
├── docs/
│   ├── architecture.md
│   ├── protocol.md
│   ├── hardware.md
│   └── images/
│
├── protocol/
│   ├── can_protocol.h
│   ├── ble_protocol.h
│   └── crc16.c
│
├── firmware/
│   ├── motion-can-node/
│   ├── env-can-node/
│   └── ble-gateway-can-node-rtos/
│
├── android/
│
└── python/

```

---

# 🎯 What This Project Demonstrates

✅ Real-time embedded system design  
✅ Multi-node CAN network  
✅ RTOS task orchestration  
✅ Binary protocol engineering  
✅ Cross-platform integration (Embedded + Android + Python)  
✅ Clean architecture principles  
✅ BLE low-level GATT implementation

---

# 🔐 Security Considerations

Current:
- No BLE pairing enforcement
- No encryption requirement
- No authentication

For production:
- BLE bonding
- Encrypted characteristics
- Secure boot on STM32
- Command authentication layer

---

# 📸 Screenshots & Diagrams

System diagrams and screenshots available in:

```
docs/images/
```

---

# 🚀 How to Run

1. Flash firmware for each STM32 node
2. Connect CAN bus (500 kbps + 120Ω termination)
3. Power all nodes
4. Connect Android app via BLE
5. Observe telemetry
6. Send commands

---

# 👨‍💻 Author

**Javier Rivera**  
Embedded Systems Engineer  
STM32 | CAN | BLE | RTOS | Android | IoT

---

# 📄 License

MIT License