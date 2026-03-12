import asyncio
import struct
from typing import List
from bleak import BleakScanner, BleakClient
from pynput import keyboard


# =====================================================
# BLE UUIDs (STM32 Custom Service)
# =====================================================
SERVICE_UUID = "0000ffe0-0000-1000-8000-00805f9b34fb"
CHAR_UUID    = "0000ffe1-0000-1000-8000-00805f9b34fb"  # Notify + Write


# =====================================================
# PROTOCOL CONSTANTS (Mirror STM32)
# =====================================================
PROTO_HEADER_1 = 0xAA
PROTO_HEADER_2 = 0x55

BLE_TYPE_MOTION = 0x01
BLE_TYPE_ENV    = 0x02
BLE_TYPE_STATUS = 0x03

STATUS_NODE_DISCONNECTED = 0x01

CMD_ALL_LED_OFF = 0x10
CMD_ALL_LED_ON  = 0x11
CMD_MOT_LED_OFF = 0x12
CMD_MOT_LED_ON  = 0x13
CMD_ENV_LED_OFF = 0x14
CMD_ENV_LED_ON  = 0x15

# =====================================================
# GLOBALS (Thread-safe bridge)
# =====================================================
command_queue = asyncio.Queue()
event_loop = None


# =====================================================
# PROTOCOL PARSER
# =====================================================
class ProtocolParser:

    def __init__(self):
        self.reset()

    def reset(self):
        self.state = "WAIT_AA"
        self.len = 0
        self.type = 0
        self.payload = []
        self.payload_idx = 0
        self.crc_bytes = []
        self.crc_buffer = []

    def process_byte(self, b):

        if self.state == "WAIT_AA":
            if b == PROTO_HEADER_1:
                self.state = "WAIT_55"

        elif self.state == "WAIT_55":
            if b == PROTO_HEADER_2:
                self.state = "WAIT_LEN"
            else:
                self.reset()

        elif self.state == "WAIT_LEN":
            self.len = b
            self.payload = []
            self.payload_idx = 0
            self.crc_buffer = [b]
            self.state = "WAIT_TYPE"

        elif self.state == "WAIT_TYPE":
            self.type = b
            self.crc_buffer.append(b)

            if self.len == 1:
                self.state = "WAIT_CRC_L"
            else:
                self.state = "WAIT_PAYLOAD"

        elif self.state == "WAIT_PAYLOAD":
            self.payload.append(b)
            self.crc_buffer.append(b)
            self.payload_idx += 1

            if self.payload_idx >= (self.len - 1):
                self.state = "WAIT_CRC_L"

        elif self.state == "WAIT_CRC_L":
            self.crc_bytes = [b]
            self.state = "WAIT_CRC_H"

        elif self.state == "WAIT_CRC_H":
            self.crc_bytes.append(b)

            crc_rx = self.crc_bytes[0] | (self.crc_bytes[1] << 8)
            crc_calc = self.crc16(self.crc_buffer)

            if crc_rx == crc_calc:
                frame = {
                    "type": self.type,
                    "payload": self.payload
                }
                self.reset()
                return frame

            print("⚠ CRC mismatch")
            self.reset()

        return None

    @staticmethod
    def crc16(data: List[int]) -> int:
        crc = 0xFFFF
        for b in data:
            crc ^= b << 8
            for _ in range(8):
                if crc & 0x8000:
                    crc = (crc << 1) ^ 0x1021
                else:
                    crc <<= 1
                crc &= 0xFFFF
        return crc


# =====================================================
# FRAME BUILDER
# =====================================================
def build_frame(frame_type: int, payload: bytes) -> bytes:

    length = 1 + len(payload)
    crc_buffer = bytes([length, frame_type]) + payload
    crc = ProtocolParser.crc16(list(crc_buffer))

    return (
        bytes([PROTO_HEADER_1, PROTO_HEADER_2, length, frame_type])
        + payload
        + bytes([crc & 0xFF, (crc >> 8) & 0xFF])
    )


# =====================================================
# FRAME DECODER
# =====================================================
def decode_frame(frame):
    frame_type = frame["type"]
    payload = bytes(frame["payload"])

    if frame_type == BLE_TYPE_MOTION and len(payload) == 5:
        node_id, timestamp = struct.unpack("<BI", payload)

        print("\n📦 MOTION FRAME")
        print(f"   Node ID   : {node_id}")
        print(f"   Timestamp : {timestamp}")

    elif frame_type == BLE_TYPE_ENV and len(payload) == 10:
        bmp_temp, aht_temp, humidity, pressure, altitude = struct.unpack("<hhHHh", payload)

        print("\n🌡 ENV FRAME")
        print(f"   BMP Temp  : {bmp_temp / 100:.2f} °C")
        print(f"   AHT Temp  : {aht_temp / 100:.2f} °C")
        print(f"   Humidity  : {humidity / 100:.2f} %")
        print(f"   Pressure  : {pressure / 10:.1f} hPa")
        print(f"   Altitude  : {altitude / 10:.1f} m")

    elif frame_type == BLE_TYPE_STATUS:
        print("\n📡 STATUS FRAME")
        print("   Raw Payload:", payload)
        if(payload[0] == STATUS_NODE_DISCONNECTED):
            if(payload[1] == BLE_TYPE_MOTION):
                print("Motion Node Disconnected")
            elif (payload[1] == BLE_TYPE_ENV):
                print("Env Node Disconnected")        

    else:
        print("\n❓ Unknown Frame Type:", frame_type)
        print("   Raw Payload:", payload)


# =====================================================
# KEYBOARD LISTENER (NON-BLOCKING)
# =====================================================
def on_press(key):
    global event_loop

    try:
        if key.char == 'q':
            asyncio.run_coroutine_threadsafe(command_queue.put("q"), event_loop)
        elif key.char in ['0','1','2','3','4','5']:
            asyncio.run_coroutine_threadsafe(command_queue.put(key.char), event_loop)
    except AttributeError:
        pass


# =====================================================
# BLE SCAN
# =====================================================
async def scan_devices():
    print("🔍 Scanning for BLE devices...\n")
    devices = await BleakScanner.discover(timeout=5.0)

    if not devices:
        print("❌ No devices found.")
        return None

    for i, device in enumerate(devices):
        name = device.name if device.name else "Unknown"
        print(f"[{i}] {name} - {device.address}")

    return devices


# =====================================================
# CONNECT + FULL DUPLEX (NON-BLOCKING)
# =====================================================
async def connect_and_listen(device):

    global event_loop
    event_loop = asyncio.get_running_loop()

    parser = ProtocolParser()

    async with BleakClient(device.address) as client:

        if not client.is_connected:
            print("❌ Failed to connect.")
            return

        print(f"✅ Connected to {device.name}")

        def notification_handler(sender, data):
            print(f"\n📩 Data: {data}")
            for byte in data:
                frame = parser.process_byte(byte)
                if frame:
                    print("✅ Valid Frame Received")
                    decode_frame(frame)

        await client.start_notify(CHAR_UUID, notification_handler)

        print("\nCommands:")
        print("   0 → ALL LED OFF")
        print("   1 → ALL LED ON")
        print("   2 → MOT LED OFF")
        print("   3 → MOT LED ON")
        print("   4 → ENV LED OFF")
        print("   5 → ENV LED ON")
        print("   q → Quit\n")

        listener = keyboard.Listener(on_press=on_press)
        listener.start()

        while True:
            cmd = await command_queue.get()

            if cmd == "q":
                break

            command_map = {
                "0": CMD_ALL_LED_OFF,
                "1": CMD_ALL_LED_ON,
                "2": CMD_MOT_LED_OFF,
                "3": CMD_MOT_LED_ON,
                "4": CMD_ENV_LED_OFF,
                "5": CMD_ENV_LED_ON,
            }

            frame = build_frame(command_map[cmd], b"")
            await client.write_gatt_char(CHAR_UUID, frame, response=False)
            print("📤 Command sent")

        listener.stop()
        await client.stop_notify(CHAR_UUID)

    print("🛑 Disconnected")


# =====================================================
# MAIN
# =====================================================
async def main():

    devices = await scan_devices()
    if not devices:
        return

    try:
        choice = int(input("\nEnter device number to connect: "))
        selected_device = devices[choice]
    except (ValueError, IndexError):
        print("❌ Invalid selection.")
        return

    await connect_and_listen(selected_device)


if __name__ == "__main__":
    asyncio.run(main())
