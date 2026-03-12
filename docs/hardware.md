# ⚙️ Hardware Overview

## 1️⃣ System Hardware Summary

The system consists of three STM32-based nodes connected over CAN bus and one BLE interface for wireless communication.

All nodes use:

- STM32F103C6T6 (Blue Pill)
- MCP2551 CAN Transceiver
- 8 MHz external crystal
- 120Ω CAN termination (bus ends only)

CAN Bitrate: **500 kbps**

---

# 2️⃣ Motion CAN Node

## Components

| Component     | Purpose          |
|---------------|------------------|
| STM32F103C6T6 | Main MCU         |
| MCP2551       | CAN transceiver  |
| HC-SR501 PIR  | Motion detection |
| LED           | Status indicator |

## Connections

### PIR (HC-SR501)

| PIR Pin | STM32 Pin              |
|---------|------------------------|
| VCC     | 5V                     |
| GND     | GND                    |
| OUT     | GPIO Input (e.g., PA0) |

### MCP2551

| MCP2551 | STM32         |
|---------|---------------|
| TXD     | CAN_TX (PA12) |
| RXD     | CAN_RX (PA11) |
| VCC     | 5V            |
| GND     | GND           |
| CANH    | CAN Bus High  |
| CANL    | CAN Bus Low   |

---

# 3️⃣ Environment CAN Node

## Components

| Component     | Purpose                |
|---------------|------------------------|
| STM32F103C6T6 | Main MCU               |
| MCP2551       | CAN transceiver        |
| BMP280        | Pressure & Temperature |
| AHT20         | Temperature & Humidity |
| LED           | Status indicator       |

## Connections

### I2C Bus

| Sensor | STM32          |
|--------|----------------|
| SDA    | PB7 (I2C1 SDA) |
| SCL    | PB6 (I2C1 SCL) |
| VCC    | 3.3V           |
| GND    | GND            |

BMP280 and AHT20 share the same I2C bus.

### MCP2551

Same wiring as Motion Node.

---

# 4️⃣ BLE Gateway Node (RTOS)

## Components

| Component     | Purpose                |
|---------------|------------------------|
| STM32F103C6T6 | Main MCU               |
| MCP2551       | CAN transceiver        |
| HM-10         | BLE 4.0 module         |
| LEDs          | Node status indicators |

## HM-10 Connections

| HM-10 | STM32            |
|-------|------------------|
| TXD   | PA10 (USART1 RX) |
| RXD   | PA9 (USART1 TX)  |
| VCC   | 3.3V             |
| GND   | GND              |

UART Configuration:

- 9600 baud
- 8N1
- No flow control

---

# 5️⃣ CAN Bus Wiring

All nodes share the same CAN bus:

```
CANH ────────────────────────────── CANH
CANL ────────────────────────────── CANL
```

Important:

- 120Ω termination resistor at both physical ends of bus
- Short stub lengths recommended
- Twisted pair wiring recommended

---

# 6️⃣ Power Considerations

- STM32 operates at 3.3V
- MCP2551 operates at 5V (logic-level compatible)
- HM-10 operates at 3.3V
- Ensure common ground across all nodes

---

# 7️⃣ Electrical Design Notes

- CAN bus designed according to ISO 11898-2
- Automatic retransmission enabled
- Auto bus-off recovery enabled
- Hardware decoupling capacitors placed near MCU and transceiver

---

# 8️⃣ Hardware Design Goals

- Low-cost development boards
- Modular nodes
- Clear separation of sensing and gateway roles
- Robust industrial communication backbone (CAN)
- Wireless monitoring via BLE

---