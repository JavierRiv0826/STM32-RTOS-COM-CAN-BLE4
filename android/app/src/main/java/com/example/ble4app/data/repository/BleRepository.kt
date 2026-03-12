package com.example.ble4app.data.repository

import com.example.ble4app.data.protocol.Frame
import kotlinx.coroutines.flow.Flow

interface BleRepository {

    suspend fun connect(address: String)
    fun observeConnectionState(): Flow<Boolean>
    suspend fun disconnect()
    fun observeNotifications(): Flow<ByteArray>
    fun observeFrames(): Flow<Frame>
    suspend fun sendCommand(commandType: Int)
}