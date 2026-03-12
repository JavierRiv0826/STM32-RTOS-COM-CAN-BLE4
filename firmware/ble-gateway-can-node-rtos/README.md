# 🚗📡 STM32 CAN ↔ BLE Gateway (FreeRTOS)

📌 Description  
A real-time embedded gateway built on the STM32F103C6T6 (Blue Pill) that bridges a 500 kbps CAN network with a BLE interface (HM-10).

The system runs on FreeRTOS (CMSIS-OS) and implements a fully asynchronous, message-driven architecture using tasks, queues, memory pools, and semaphores.

The gateway:
* Receives CAN frames from distributed nodes
* Reassembles fragmented packets
* Detects offline nodes
* Forwards telemetry to BLE
* Receives BLE commands
* Sends control commands back over CAN

This project demonstrates real-time system design, multi-protocol bridging, packet reassembly, supervision logic, and RTOS-based concurrency management.

<p align="center">
  <img src="/docs/images/gateway-can-node.jpg" width="1000"/>
  <br>
  <em>STM32 Blue Pill BLE Gateway Node with CAN Bus Interface</em>
</p>

---

# 🧠 Real-Time Architecture

The firmware is fully event-driven.

## 🧵 Tasks

| Task            | Priority    | Responsibility                                    |
|-----------------|-------------|---------------------------------------------------|
| CanRxTask       | AboveNormal | Receives CAN frames and forwards to gateway queue |
| BleRxTask       | AboveNormal | Collects incoming BLE bytes (UART driven)         |
| GatewayCANTask  | Normal      | Processes CAN frames                              |
| GatewayBLETask  | Normal      | Processes BLE protocol frames                     |
| SupervisionTask | Low         | Offline detection & supervision                   |
| DefaultTask     | Normal      | Idle placeholder                                  |

---

## 📦 Inter-Task Communication

The system uses:

* ✔ Message Queues
* ✔ Memory Pools
* ✔ Binary Semaphore
* ✔ CMSIS-OS Scheduler

### Queues

- `q_can_rx` → ISR → CanRxTask
- `q_gw_can` → CanRxTask → GatewayCANTask
- `q_gw_ble` → BleRxTask → GatewayBLETask

### Memory Pool

Dynamic allocation avoided —  
CAN frames are allocated from a fixed RTOS memory pool:

```
osPoolDef(canPool, 8, can_frame_t);
```

This ensures:
* Deterministic memory usage
* No heap fragmentation
* Real-time safety

---

# ⚙️ STM32 Configuration

## 🔄 Clock

* HSE = 8 MHz
* PLL ×9 → SYSCLK = 72 MHz
* APB1 = 36 MHz
* APB2 = 72 MHz

---

## 🚗 CAN (500 kbps)

* Prescaler = 4
* BS1 = 13 TQ
* BS2 = 4 TQ
* SJW = 1 TQ
* Auto Bus-Off = Enabled
* Auto Wake-Up = Enabled
* Auto Retransmission = Enabled

---

## 📡 UART (HM-10 BLE)

* USART1
* 9600 baud
* 8N1
* Interrupt-driven RX
* Non-blocking transmission

The HM-10 operates in transparent UART mode.

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

## 🔌 Wiring HM-10 BLE Module

The gateway communicates with the HM-10 using UART in transparent mode.

### Connections

| HM-10 | STM32F103        |
|-------|------------------|
| VCC   | 3.3V             |
| GND   | GND              |
| TXD   | PA10 (USART1_RX) |
| RXD   | PA9 (USART1_TX)  |

Configuration:
- Baud rate: 9600
- Mode: Transparent UART
- 8N1 format

⚠ Important:
- Use 3.3V logic levels
- If using a 5V HM-10 breakout, verify voltage compatibility

---

## 💡 Status LEDs

| Pin  | Function                          |
|------|-----------------------------------|
| PC13 | BLE activity indicator            |
| PB3  | Motion Node Online indicator      |
| PB4  | Environment Node Online indicator |

---

# 🔁 CAN Processing

## Motion Node

* Validates protocol version
* Extracts motion payload
* Forwards as BLE_TYPE_MOTION
* Updates supervision timestamp

## Environment Node

Implements fragment reassembly:

* Receives 2 fragments
* Each fragment carries 5 bytes
* Uses fragment bitmask tracking
* Reassembles 10-byte sensor packet
* Forwards as BLE_TYPE_ENV

This demonstrates safe multi-frame packet reconstruction over CAN.

---

# 🔎 Offline Supervision

Each node maintains:

```
node_status_t
{
    node_id
    last_seen_ms
    online flag
}
```

If no frame is received for:

```
OFFLINE_TIMEOUT_MS = 15000
```

The gateway:

* Marks node offline
* Turns off status LED
* Sends BLE status frame:
    - STATUS_NODE_DISCONNECTED
    - NODE_TYPE_MOTION or NODE_TYPE_ENV

This adds distributed system fault detection.

---

# 📡 BLE Protocol

Custom framed protocol:

```
[0xAA][0x55][LEN][TYPE][PAYLOAD...][CRC16_L][CRC16_H]
```

Features:

* Stateful byte parser
* CRC16 validation
* Framed binary transport
* Bidirectional communication

Types:

| Type            | Meaning                 |
|-----------------|-------------------------|
| BLE_TYPE_MOTION | Motion telemetry        |
| BLE_TYPE_ENV    | Environmental telemetry |
| BLE_TYPE_STATUS | System status           |

---

# 🎛 Command Handling

Commands received via BLE are translated into CAN commands.

Examples:

| BLE Command   | CAN Action              |
|---------------|-------------------------|
| ALL LEDs OFF  | Broadcast CMD_LED_OFF   |
| Motion LED ON | Target Motion Node      |
| Env LED OFF   | Target Environment Node |

The gateway builds a CAN protocol frame and transmits it over the bus.

---

# 🏗 Firmware Architecture

```
+----------------------------+
| Gateway Application Layer  |
| - CAN handler              |
| - BLE handler              |
| - Reassembly logic         |
| - Supervision              |
| - Command translation      |
+----------------------------+
            ↓
+----------------------------+
| Protocol Layer             |
| - CAN protocol             |
| - BLE framed protocol      |
| - CRC16                    |
+----------------------------+
            ↓
+----------------------------+
| Drivers                    |
| - CAN HAL wrapper          |
| - UART BLE driver          |
+----------------------------+
            ↓
+----------------------------+
| STM32 HAL + FreeRTOS       |
+----------------------------+
```

---

# 🗂 Project Structure

```bash
project/
├── Core/
│   └── Src/
│       └── main.c
│
├── Gateway/
│   ├── gateway.c
│   └── gateway.h
│
├── CAN/
│   ├── can_driver.c
│   └── can_driver.h
│   └── protocol/
│       └── protocol.h
│
├── BLE/
│   ├── ble_driver.c
│   ├── ble_driver.h
│   ├── ble_types.h
│   └── protocol/
│       └── ble_protocol.h
│
└── utils/
    ├── crc16.c
    └── crc16.h
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

### On CAN Reception

- CAN frames received via interrupt
- Forwarded through RTOS queue
- Gateway reassembles ENV packets
- Motion packets forwarded immediately
- BLE frame sent via UART

### On BLE Command

If BLE command is received:

- Parsed via state machine
- Converted to CAN protocol frame
- Sent to target node
- Node LED state changes

---

## 🟢 Online Indicators

| LED  | Meaning                 |
|------|-------------------------|
| PB3  | Motion Node Online      |
| PB4  | Environment Node Online |
| PC13 | BLE activity            |

---

## 🔴 Offline Detection

If no frame is received from a node for:

```
15 seconds
```

Gateway:

- Turns OFF corresponding LED
- Sends BLE status frame:
    - STATUS_NODE_DISCONNECTED
    - NODE_TYPE_MOTION or NODE_TYPE_ENV

---

# 🧠 Engineering Highlights

* Full FreeRTOS integration
* Deterministic memory pool usage
* Interrupt-safe queue communication
* Multi-frame CAN reassembly
* Offline node detection
* Custom framed BLE protocol
* Bidirectional command bridge
* Layered modular architecture

---

# 🛠 Development Tools

* STM32CubeIDE
* STM32CubeMX
* FreeRTOS (CMSIS-OS wrapper)

---

# 👤 Author

Javier Rivera  
Embedded Systems Engineer  
GitHub: JavierRiv0826