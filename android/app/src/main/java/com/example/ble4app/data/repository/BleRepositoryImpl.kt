package com.example.ble4app.data.repository

import android.util.Log
import com.example.ble4app.data.ble.BleDataSource
import com.example.ble4app.data.protocol.Frame
import com.example.ble4app.data.protocol.FrameBuilder
import com.example.ble4app.data.protocol.ProtocolParser
import kotlinx.coroutines.ExperimentalCoroutinesApi
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.flatMapConcat
import kotlinx.coroutines.flow.flow
import javax.inject.Inject
import javax.inject.Singleton

@Singleton
class BleRepositoryImpl @Inject constructor(
    private val bleDataSource: BleDataSource
) : BleRepository {

    private val parser = ProtocolParser()

    override suspend fun connect(address: String) {
        bleDataSource.connect(address)
    }

    override fun observeConnectionState(): Flow<Boolean> {
        return bleDataSource.observeConnectionState()
    }

    override suspend fun disconnect() {
        bleDataSource.disconnect()
    }

    override fun observeNotifications(): Flow<ByteArray> {
        return bleDataSource.observeNotifications()
    }

    override fun observeFrames(): Flow<Frame> = flow {

        bleDataSource.observeNotifications()
            .collect { byteArray ->

                for (byte in byteArray) {
                    val frame = parser.process(byte)
                    if (frame != null) {
                        Log.d("BLE_PARSER", "Frame parsed: $frame")
                        emit(frame)
                    }
                }
            }
    }

    override suspend fun sendCommand(commandType: Int) {

        val frame = FrameBuilder.build(commandType)

        bleDataSource.write(frame)
    }
}