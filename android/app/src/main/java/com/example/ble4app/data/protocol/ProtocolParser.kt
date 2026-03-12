package com.example.ble4app.data.protocol

import android.util.Log

class ProtocolParser {

    private enum class State {
        WAIT_AA,
        WAIT_55,
        WAIT_LEN,
        WAIT_TYPE,
        WAIT_PAYLOAD,
        WAIT_CRC_L,
        WAIT_CRC_H
    }

    private var state = State.WAIT_AA
    private var length = 0
    private var type = 0
    private val payload = mutableListOf<Byte>()
    private val crcBuffer = mutableListOf<Byte>()
    private var crcLow = 0

    fun process(byte: Byte): Frame? {

        val b = byte.toInt() and 0xFF

        Log.d("BLE_PARSER", "Byte: ${b.toInt() and 0xFF}")

        when (state) {

            State.WAIT_AA -> {
                if (b == 0xAA) state = State.WAIT_55
            }

            State.WAIT_55 -> {
                if (b == 0x55) {
                    state = State.WAIT_LEN
                } else {
                    reset()
                }
            }

            State.WAIT_LEN -> {
                length = b
                payload.clear()
                crcBuffer.clear()
                crcBuffer.add(byte)
                state = State.WAIT_TYPE
            }

            State.WAIT_TYPE -> {
                type = b
                crcBuffer.add(byte)

                state = if (length == 1)
                    State.WAIT_CRC_L
                else
                    State.WAIT_PAYLOAD
            }

            State.WAIT_PAYLOAD -> {
                payload.add(byte)
                crcBuffer.add(byte)

                if (payload.size >= (length - 1)) {
                    state = State.WAIT_CRC_L
                }
            }

            State.WAIT_CRC_L -> {
                crcLow = b
                state = State.WAIT_CRC_H
            }

            State.WAIT_CRC_H -> {

                val crcHigh = b
                val receivedCrc = crcLow or (crcHigh shl 8)

                val calculatedCrc =
                    Crc16.compute(crcBuffer.toByteArray())

                if (receivedCrc == calculatedCrc) {
                    Log.d("BLE_PARSER", "CRC OK")
                    val frame = decodeFrame()
                    reset()
                    return frame
                } else {
                    Log.d("BLE_PARSER", "CRC MISMATCH: received=$receivedCrc calc=$calculatedCrc")
                    reset()
                }
            }
        }

        return null
    }

    private fun decodeFrame(): Frame? {

        return when (type) {

            0x01 -> { // MOTION
                if (payload.size == 5) {

                    val nodeId = payload[0].toInt() and 0xFF

                    val timestamp =
                        (payload[1].toLong() and 0xFF) or
                                ((payload[2].toLong() and 0xFF) shl 8) or
                                ((payload[3].toLong() and 0xFF) shl 16) or
                                ((payload[4].toLong() and 0xFF) shl 24)

                    Frame.Motion(nodeId, timestamp)
                } else null
            }

            0x02 -> { // ENV
                if (payload.size == 10) {

                    fun shortAt(i: Int): Short {
                        return ((payload[i].toInt() and 0xFF) or
                                ((payload[i + 1].toInt() and 0xFF) shl 8)).toShort()
                    }

                    val bmpTemp = shortAt(0) / 100f
                    val ahtTemp = shortAt(2) / 100f
                    val humidity = shortAt(4).toInt() / 100f
                    val pressure = shortAt(6).toInt() / 10f
                    val altitude = shortAt(8) / 10f

                    Frame.Env(
                        bmpTemp,
                        ahtTemp,
                        humidity,
                        pressure,
                        altitude
                    )
                } else null
            }

            0x03 -> { // STATUS
                if (payload.size >= 2) {
                    Frame.Status(
                        statusType = payload[0].toInt() and 0xFF,
                        nodeType = payload[1].toInt() and 0xFF
                    )
                } else null
            }

            else -> null
        }
    }

    private fun reset() {
        state = State.WAIT_AA
        payload.clear()
        crcBuffer.clear()
    }
}