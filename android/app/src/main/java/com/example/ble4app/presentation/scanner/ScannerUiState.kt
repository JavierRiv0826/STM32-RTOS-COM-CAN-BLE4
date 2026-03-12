package com.example.ble4app.presentation.scanner

import com.example.ble4app.data.ble.BleDevice

data class ScannerUiState(
    val isScanning: Boolean = false,
    val devices: List<BleDevice> = emptyList(),
    val error: String? = null
)