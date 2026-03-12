package com.example.ble4app.data.ble

import kotlinx.coroutines.flow.Flow

interface BleDataSource {

    fun scan(): Flow<BleDevice>
    suspend fun connect(address: String)
    fun observeConnectionState(): Flow<Boolean>
    suspend fun disconnect()
    fun observeNotifications(): Flow<ByteArray>
    suspend fun write(data: ByteArray)
}