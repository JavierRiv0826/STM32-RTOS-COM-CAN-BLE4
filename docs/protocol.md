# 📡 System Communication Flow

## 1️⃣ Overview

This document explains how data flows through the system and how nodes interact over:

- CAN Bus (500 kbps)
- BLE (HM-10, UART transparent mode)

For low-level frame definitions, see:

```
/protocol/
```

This document focuses on **runtime behavior and message flow**.

---

# 2️⃣ Forward Path – Motion Event

### Step-by-step flow:

1. PIR sensor detects motion.
2. Motion Node builds CAN frame (MSG_MOTION).
3. Frame transmitted on CAN (ID = 0x100).
4. Gateway receives frame (CanRxTask).
5. Frame forwarded to GatewayCANTask.
6. Gateway builds BLE frame (TYPE = BLE_TYPE_MOTION).
7. Frame sent over UART to HM-10.
8. Android receives and parses frame.

### Data Flow Diagram

PIR → STM32 Motion → CAN → Gateway → BLE → Android

---

# 3️⃣ Forward Path – Environmental Telemetry

Environmental data exceeds 5-byte CAN payload capacity.

### Fragmentation Strategy

- fragment_idx = 0 → first 5 bytes
- fragment_idx = 1 → last 5 bytes

### Flow:

1. Sensors sampled (BMP280 + AHT20).
2. Node builds full 10-byte packet.
3. Packet split into 2 CAN frames.
4. Gateway receives fragments.
5. Gateway reassembles packet.
6. BLE frame built.
7. Data sent to Android.

### Reassembly Logic

Gateway tracks:

- Active packet state
- Fragments received (bitmask)
- 10-byte buffer

When both fragments received:

→ Packet forwarded over BLE.

---

# 4️⃣ Reverse Path – Command Flow

Commands originate from Android.

### Flow:

1. Android sends BLE frame.
2. GatewayBLETask parses frame.
3. Command mapped to CAN MSG_COMMAND.
4. CAN frame transmitted to target node.
5. Node executes command (e.g., LED ON/OFF).

### Target Routing

| Target | Value |
|--------|-------|
| All Nodes | 1 |
| Motion Node | 2 |
| Environment Node | 3 |

---

# 5️⃣ Heartbeat Mechanism

Each CAN node periodically sends:

MSG_HEARTBEAT

Purpose:

- Confirm node is alive
- Update last_seen timestamp

Gateway does not forward heartbeat unless required.

---

# 6️⃣ Offline Supervision

Gateway tracks last received frame timestamp.

Offline timeout:

15,000 ms

If timeout exceeded:

- Node marked offline
- Status LED turned OFF
- BLE status frame sent to Android

Status payload:

```
STATUS_NODE_DISCONNECTED
NODE_TYPE_X
```

---

# 7️⃣ Gateway Internal Flow (RTOS)

## CAN Path

CAN Interrupt → q_can_rx → CanRxTask → q_gw_can → GatewayCANTask → BLE

## BLE Path

UART RX Interrupt → Semaphore → BleRxTask → q_gw_ble → GatewayBLETask → CAN

RTOS ensures:

- Deterministic task separation
- Non-blocking communication
- Memory pool usage for CAN frames

---

# 8️⃣ Reliability Mechanisms

CAN Layer:
- Automatic retransmission enabled
- Auto bus-off recovery
- Hardware filtering

BLE Layer:
- Framed protocol
- CRC16-CCITT validation
- Stateful parser

Application Layer:
- Heartbeat monitoring
- Offline detection
- Fragment validation

---

# 9️⃣ Design Decisions

## Why CAN?

- Deterministic
- Robust
- Automotive-grade reliability
- Built-in arbitration

## Why 500 kbps?

- Balanced speed vs stability
- Suitable for short bus topology
- Industry common configuration

## Why Fragmentation?

- CAN payload limited to 8 bytes
- Environmental data exceeds 5 usable bytes

## Why BLE framing with CRC?

- HM-10 provides raw UART stream
- Custom framing ensures synchronization
- CRC prevents corrupted commands

---

# 🔟 Scalability

The system supports:

- Additional CAN nodes
- New message types
- Protocol version extension
- Increased telemetry types
- Encrypted BLE transport layer

The separation between:

Transport → Protocol → Application → RTOS Tasks

enables clean extensibility.

---