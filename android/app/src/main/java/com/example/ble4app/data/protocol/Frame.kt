package com.example.ble4app.data.protocol

sealed class Frame {

    data class Motion(
        val nodeId: Int,
        val timestamp: Long
    ) : Frame()

    data class Env(
        val bmpTemp: Float,
        val ahtTemp: Float,
        val humidity: Float,
        val pressure: Float,
        val altitude: Float
    ) : Frame()

    data class Status(
        val statusType: Int,
        val nodeType: Int
    ) : Frame()
}