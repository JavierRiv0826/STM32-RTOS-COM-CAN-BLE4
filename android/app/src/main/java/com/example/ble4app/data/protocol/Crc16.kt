package com.example.ble4app.data.protocol

object Crc16 {

    fun compute(data: ByteArray): Int {

        var crc = 0xFFFF

        for (byte in data) {
            crc = crc xor ((byte.toInt() and 0xFF) shl 8)

            repeat(8) {
                crc = if (crc and 0x8000 != 0) {
                    (crc shl 1) xor 0x1021
                } else {
                    crc shl 1
                }
                crc = crc and 0xFFFF
            }
        }

        return crc and 0xFFFF
    }
}