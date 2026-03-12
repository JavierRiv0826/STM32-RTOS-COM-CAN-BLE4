package com.example.ble4app.data.protocol

object FrameBuilder {

    private const val HEADER_1 = 0xAA
    private const val HEADER_2 = 0x55

    fun build(frameType: Int, payload: ByteArray = byteArrayOf()): ByteArray {

        val length = 1 + payload.size

        val crcBuffer = ByteArray(2 + payload.size)
        crcBuffer[0] = length.toByte()
        crcBuffer[1] = frameType.toByte()
        payload.copyInto(crcBuffer, 2)

        val crc = crc16(crcBuffer)

        return buildList {
            add(HEADER_1.toByte())
            add(HEADER_2.toByte())
            add(length.toByte())
            add(frameType.toByte())
            addAll(payload.toList())
            add((crc and 0xFF).toByte())
            add(((crc shr 8) and 0xFF).toByte())
        }.toByteArray()
    }

    private fun crc16(data: ByteArray): Int {

        var crc = 0xFFFF

        for (byte in data) {
            val value = byte.toInt() and 0xFF
            crc = crc xor (value shl 8)

            repeat(8) {
                crc = if ((crc and 0x8000) != 0) {
                    (crc shl 1) xor 0x1021
                } else {
                    crc shl 1
                }
                crc = crc and 0xFFFF
            }
        }

        return crc
    }
}