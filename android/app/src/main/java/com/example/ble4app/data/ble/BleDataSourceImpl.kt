package com.example.ble4app.data.ble

import android.Manifest
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothGatt
import android.bluetooth.BluetoothGattCallback
import android.bluetooth.BluetoothGattCharacteristic
import android.bluetooth.BluetoothGattDescriptor
import android.bluetooth.BluetoothProfile
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanResult
import android.content.Context
import android.content.pm.PackageManager
import android.os.Build
import android.util.Log
import androidx.core.content.ContextCompat
import dagger.hilt.android.qualifiers.ApplicationContext
import kotlinx.coroutines.channels.BufferOverflow
import kotlinx.coroutines.channels.awaitClose
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.MutableSharedFlow
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.asSharedFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.callbackFlow
import java.util.UUID
import javax.inject.Inject
import javax.inject.Singleton

@Singleton
class BleDataSourceImpl @Inject constructor(
    @ApplicationContext private val context: Context,
    private val bluetoothAdapter: BluetoothAdapter
) : BleDataSource {

    private val TAG = "BLE_DEBUG"

    private val SERVICE_UUID =
        UUID.fromString("0000ffe0-0000-1000-8000-00805f9b34fb")

    private val CHAR_UUID =
        UUID.fromString("0000ffe1-0000-1000-8000-00805f9b34fb")

    private val CLIENT_CONFIG_UUID =
        UUID.fromString("00002902-0000-1000-8000-00805f9b34fb")

    // ✅ BUFFERED SharedFlow (critical for BLE)
    private val _notifications = MutableSharedFlow<ByteArray>(
        replay = 0,
        extraBufferCapacity = 128,
        onBufferOverflow = BufferOverflow.DROP_OLDEST
    )

    private var bluetoothGatt: BluetoothGatt? = null
    private val _connectionState = MutableStateFlow(false)

    private val gattCallback = object : BluetoothGattCallback() {

        override fun onConnectionStateChange(
            gatt: BluetoothGatt,
            status: Int,
            newState: Int
        ) {
            Log.d(TAG, "onConnectionStateChange: status=$status newState=$newState")

            if (newState == BluetoothProfile.STATE_CONNECTED) {
                Log.d(TAG, "Connected → Discovering services")
                _connectionState.value = true
                gatt.discoverServices()
            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                Log.d(TAG, "Disconnected")
                _connectionState.value = false
            }
        }

        override fun onServicesDiscovered(
            gatt: BluetoothGatt,
            status: Int
        ) {
            Log.d(TAG, "onServicesDiscovered: status=$status")

            if (status != BluetoothGatt.GATT_SUCCESS) {
                Log.d(TAG, "Service discovery failed")
                return
            }

            gatt.services.forEach {
                Log.d(TAG, "Service found: ${it.uuid}")
            }

            val service = gatt.getService(SERVICE_UUID)
            if (service == null) {
                Log.d(TAG, "STM32 Service NOT found")
                return
            }

            Log.d(TAG, "STM32 Service found")

            val characteristic = service.getCharacteristic(CHAR_UUID)
            if (characteristic == null) {
                Log.d(TAG, "STM32 Characteristic NOT found")
                return
            }

            Log.d(TAG, "STM32 Characteristic found")

            enableNotifications(gatt, characteristic)
        }

        override fun onDescriptorWrite(
            gatt: BluetoothGatt,
            descriptor: BluetoothGattDescriptor,
            status: Int
        ) {
            Log.d(TAG, "onDescriptorWrite: status=$status")
        }

        override fun onCharacteristicChanged(
            gatt: BluetoothGatt,
            characteristic: BluetoothGattCharacteristic
        ) {
            if (characteristic.uuid == CHAR_UUID) {

                val data = characteristic.value

                Log.d(TAG, "Notification received!")
                Log.d(TAG, "Raw data: ${data.joinToString()}")

                val emitted = _notifications.tryEmit(data)
                Log.d(TAG, "Notification emitted: $emitted")
            }
        }
    }

    override fun scan(): Flow<BleDevice> = callbackFlow {

        val scanner = bluetoothAdapter.bluetoothLeScanner

        val callback = object : ScanCallback() {

            override fun onScanResult(
                callbackType: Int,
                result: ScanResult
            ) {
                val device = result.device

                val safeName = if (
                    Build.VERSION.SDK_INT < Build.VERSION_CODES.S ||
                    ContextCompat.checkSelfPermission(
                        context,
                        Manifest.permission.BLUETOOTH_CONNECT
                    ) == PackageManager.PERMISSION_GRANTED
                ) {
                    device.name
                } else {
                    null
                }

                trySend(
                    BleDevice(
                        name = safeName,
                        address = device.address
                    )
                )
            }

            override fun onScanFailed(errorCode: Int) {
                close(Exception("Scan failed: $errorCode"))
            }
        }

        scanner.startScan(callback)

        awaitClose {
            scanner.stopScan(callback)
        }
    }

    override fun observeConnectionState(): Flow<Boolean> =
        _connectionState.asStateFlow()

    override suspend fun connect(address: String) {

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S &&
            ContextCompat.checkSelfPermission(
                context,
                Manifest.permission.BLUETOOTH_CONNECT
            ) != PackageManager.PERMISSION_GRANTED
        ) {
            throw SecurityException("Missing BLUETOOTH_CONNECT permission")
        }

        val device = bluetoothAdapter.getRemoteDevice(address)

        bluetoothGatt = device.connectGatt(
            context,
            false,
            gattCallback
        )
    }

    override suspend fun disconnect() {
        bluetoothGatt?.close()
        bluetoothGatt = null
        _connectionState.value = false
    }

    override fun observeNotifications(): Flow<ByteArray> =
        _notifications.asSharedFlow()

    private fun enableNotifications(
        gatt: BluetoothGatt,
        characteristic: BluetoothGattCharacteristic
    ) {

        Log.d(TAG, "Enabling notifications...")

        val success = gatt.setCharacteristicNotification(characteristic, true)
        Log.d(TAG, "setCharacteristicNotification result: $success")

        val descriptor = characteristic.getDescriptor(CLIENT_CONFIG_UUID)
        if (descriptor == null) {
            Log.d(TAG, "Descriptor 0x2902 NOT found")
            return
        }

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            val result = gatt.writeDescriptor(
                descriptor,
                BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE
            )
            Log.d(TAG, "writeDescriptor (API33+) result: $result")
        } else {
            descriptor.value = BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE
            val result = gatt.writeDescriptor(descriptor)
            Log.d(TAG, "writeDescriptor result: $result")
        }
    }

    override suspend fun write(data: ByteArray) {

        val gatt = bluetoothGatt ?: return

        val service = gatt.getService(SERVICE_UUID) ?: return
        val characteristic = service.getCharacteristic(CHAR_UUID) ?: return

        characteristic.value = data

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            gatt.writeCharacteristic(
                characteristic,
                data,
                BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT
            )
        } else {
            characteristic.writeType = BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT
            gatt.writeCharacteristic(characteristic)
        }

        Log.d(TAG, "Write sent: ${data.joinToString()}")
    }
}