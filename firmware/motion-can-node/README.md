# 🚗 STM32 CAN Motion Node

📌 Description  
A modular embedded node built on the STM32F103C6T6 (Blue Pill) featuring motion detection using a PIR HC-SR501 sensor and real-time communication over a 500 kbps CAN bus.

This node is part of a distributed CAN network and is responsible for publishing motion events and periodic heartbeat messages to a BLE gateway.

The firmware follows a layered architecture for clean separation of concerns:
* Motion handling (PIR + EXTI interrupt)
* CAN driver abstraction
* Application layer (event processing, heartbeat, command handling)

This project demonstrates deterministic embedded design, interrupt-driven event handling, and scalable CAN-based distributed systems.

<p align="center">
  <img src="/docs/images/motion-can-node.jpg" width="1000"/>
  <br>
  <em>STM32 Blue Pill Motion Detection Node with CAN Bus Interface</em>
</p>

---

## 🚀 Features

### 🕵️ Motion Detection
* ✔ PIR HC-SR501 sensor on PB9 (EXTI rising edge)
* ✔ Immediate interrupt-driven motion event
* ✔ Timestamped using TIM2 (1 kHz)
* ✔ LED feedback (PC13)

### 🚗 CAN Communication (500 kbps)
* ✔ Standard 11-bit CAN identifiers
* ✔ Auto retransmission enabled
* ✔ Event-driven motion frame transmission
* ✔ Periodic heartbeat frame
* ✔ Command reception (e.g., LED control)

### 🧠 CAN Bus Physical Layer

* Standard: ISO 11898-2 (High-Speed CAN)
* Bitrate: 500 kbps
* Differential signaling (CANH/CANL)
* 120Ω termination at both ends
* Multi-node bus topology

### 🧠 Application Layer
* ✔ Rolling frame counter
* ✔ Unified 8-byte protocol frame
* ✔ Command parsing (target + command)
* ✔ Non-blocking main loop

---

# ⚙️ STM32CubeMX Configuration

## 🔄 Clock
* HSE = 8 MHz
* PLL ×9 → SYSCLK = 72 MHz
* AHB = 72 MHz
* APB1 = 36 MHz
* APB2 = 72 MHz

Optimized to support accurate CAN timing at 500 kbps.

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

## 🔁 TIM2 (System Timer)
* Prescaler = 71 → 1 ms tick
* Period = 999
* Used for:
    * Motion timestamp
    * Heartbeat timing

---

## ↔️ GPIO Configuration
* PB9 → PIR sensor (EXTI rising)
* PC13 → Status LED
* PB3 → Command-controlled LED

---

## 🔌 Wiring (PIR HC-SR501 → STM32)

| PIR | STM32F103 |
|-----|-----------|
| VCC | 5V        |
| GND | GND       |
| OUT | PB9       |

⚠️ PB9 configured as GPIO_MODE_IT_RISING with pulldown resistor.

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

⚠️ Notes:
* STM32 CAN pins:
  * PA11 → CAN_RX
  * PA12 → CAN_TX
* 120Ω termination resistor required at both ends of the CAN bus.
* MCP2551 operates at 5V, but TXD/RXD are 3.3V compatible with STM32.

---

## 📦 Protocol Overview

All CAN frames use a unified 8-byte structure:

| Byte | Description                              |
|------|------------------------------------------|
| 0    | Version (2 bits) + Message Type (6 bits) |
| 1    | Rolling Counter                          |
| 2    | Fragment Index (0 = not fragmented)      |
| 3–7  | Payload (5 bytes)                        |

### Message Types

| Type          | Value |
|---------------|-------|
| MSG_HEARTBEAT | 1     |
| MSG_MOTION    | 2     |
| MSG_ENV       | 3     |
| MSG_COMMAND   | 4     |

### Motion Payload (5 Bytes)

| Byte | Description             |
|------|-------------------------|
| 0    | Node ID                 |
| 1–4  | Timestamp (uint32_t ms) |

---

## 🔐 Heartbeat Mechanism

The node periodically sends a heartbeat frame to indicate presence on the network.

This allows the gateway to:
* Detect node disconnection
* Monitor network health
* Validate communication integrity

---

## 🧩 Drivers Overview

### app.c / app.h
* Handles motion trigger logic
* Assembles protocol frames
* Manages rolling counter
* Sends heartbeat

### can_driver.c / can_driver.h
* Hardware abstraction layer for CAN
* Functions:
    * CAN_Driver_Init()
    * CAN_Driver_Send()
    * CAN_Driver_GetRx()

### protocol.h
* Unified CAN frame structure
* Message type definitions
* Payload structures

---

## 🏗 Firmware Architecture

```text
+----------------------------+
| Application Layer          |
| - Event handling           |
| - Heartbeat generation     |
| - Command processing       |
+----------------------------+
             ↓
+----------------------------+
| CAN Driver                 |
| - Frame transmit           |
| - Frame receive            |
| - HAL abstraction          |
+----------------------------+
             ↓
+----------------------------+
| STM32 HAL + Hardware       |
| - EXTI (PB9)               |
| - TIM2 (1ms tick)          |
| - CAN1 peripheral          |
+----------------------------+
```

---

## 🗂 Project Structure

```bash
project/
├── Core/
│   ├── Inc/
│   └── Src/
│       ├── main.c
│       └── stm32f1xx_it.c
│
├── CAN/
│   ├── can_driver.c
│   └── can_driver.h
│
├── app/
│   ├── app.c
│   └── app.h
│
└── protocol/
    └── protocol.h
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
### 6. Trigger motion → Frame transmitted on CAN
### 7. Observe heartbeat frames periodically

---

## 🧠 Engineering Highlights

* Interrupt-driven event detection
* Deterministic non-RTOS design
* Custom CAN protocol implementation
* Rolling counter for traceability
* Distributed multi-node architecture

---

## 🛠 Development Tools
* STM32CubeIDE
* STM32CubeMX

---

## 👤 Author
Javier Rivera  
GitHub: JavierRiv0826