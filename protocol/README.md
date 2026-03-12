# 📡 CAN + BLE Communication Protocol Specification

## 1️⃣ Overview

This document defines the binary communication protocol used in:

- 🚗 CAN Motion Node
- 🌡️ CAN Environment Node
- 🧠 RTOS BLE Gateway
- 📱 Android BLE Application
- 🐍 Python Debug Tool

### Transport Layers

| Layer     | Technology                             |
|-----------|----------------------------------------|
| Field Bus | CAN 2.0A (500 kbps)                    |
| Wireless  | BLE 4.0 (HM-10, UART Transparent Mode) |
| UART      | 9600 baud                              |

---

# 2️⃣ CAN Protocol

All CAN frames use:

- Standard ID (11-bit)
- Fixed 8-byte payload
- Deterministic frame size

Bitrate: **500 kbps**

---

## 📦 CAN Frame Layout (8 Bytes)

| Byte | Description                              |
|------|------------------------------------------|
| 0    | Version (2 bits) + Message Type (6 bits) |
| 1    | Frame Counter                            |
| 2    | Fragment Index                           |
| 3–7  | Payload (5 bytes)                        |

### Byte 0 (ver_type)

```
[7:6] = Protocol Version
[5:0] = Message Type
```

---

## 3️⃣ CAN IDs

| Node             | CAN ID |
|------------------|--------|
| Motion Node      | 0x100  |
| Environment Node | 0x200  |
| Gateway          | 0x300  |

---

## 4️⃣ Message Types

| Type | Description             |
|------|-------------------------|
| 1    | Heartbeat               |
| 2    | Motion Event            |
| 3    | Environmental Telemetry |
| 4    | Command                 |

---

## 5️⃣ Motion Payload (MSG_MOTION)

| Field     | Type   |
|-----------|--------|
| node_id   | uint8  |
| timestamp | uint32 |

Fits in single CAN frame.

---

## 6️⃣ Environmental Payload (MSG_ENV)

Because environmental data exceeds 5 bytes, it is fragmented.

### Fragmentation:

- fragment_idx = 0 → first 5 bytes
- fragment_idx = 1 → last 5 bytes
- Gateway reassembles packet

### Complete Packet (10 bytes)

| Field         | Type   | Unit    |
|---------------|--------|---------|
| bmp_temp_x100 | int16  | °C ×100 |
| aht_temp_x100 | int16  | °C ×100 |
| humidity_x100 | uint16 | % ×100  |
| pressure_dpa  | uint16 | dPa     |
| altitude_x10  | int16  | m ×10   |

---

## 7️⃣ Command Payload (MSG_COMMAND)

| Field   | Type     |
|---------|----------|
| target  | uint8    |
| command | uint8    |
| data[3] | reserved |

Targets:

| Target      | Value |
|-------------|-------|
| All Nodes   | 1     |
| Motion      | 2     |
| Environment | 3     |

Commands:

| Command | Value |
|---------|-------|
| LED OFF | 0     |
| LED ON  | 1     |

---

# 8️⃣ BLE Transport Protocol

BLE uses a framed binary protocol over UART (HM-10).

Frame format:

```
| 0xAA | 0x55 | LEN | TYPE | PAYLOAD | CRC16 |
```

### CRC calculated over:
```
LEN + TYPE + PAYLOAD
```

SOF bytes are NOT included in CRC.

---

## 9️⃣ CRC16-CCITT Specification

| Parameter     | Value     |
|---------------|-----------|
| Polynomial    | 0x1021    |
| Initial Value | 0xFFFF    |
| Reflection    | No        |
| Final XOR     | 0x0000    |
| Output        | MSB First |

Test Vector:

```
Input: "123456789"
CRC: 0x29B1
```

---

# 🔟 Supervision & Fault Detection

Gateway tracks:

- Node heartbeat
- Last frame timestamp
- Offline timeout: 15000 ms

If node disconnects:
- LED turned OFF
- BLE status frame sent

---

# 1️⃣1️⃣ Versioning

Current Protocol Version:

```
PROTOCOL_VERSION = 1
```

Future compatibility handled via:
- 2-bit version field
- Backward-compatible parsing

---

# 1️⃣2️⃣ Design Goals

- Deterministic CAN frame size
- Fragmentation support
- Lightweight parsing
- RTOS-friendly architecture
- Cross-platform BLE framing
- CRC-protected communication
- Extensible versioning

---

# 📄 License

MIT License

---

## 👨‍💻 Author

Javier Rivera  
GitHub: JavierRiv0826