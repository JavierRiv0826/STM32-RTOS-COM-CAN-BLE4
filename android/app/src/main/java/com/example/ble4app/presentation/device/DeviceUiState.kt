package com.example.ble4app.presentation.device

import com.example.ble4app.data.protocol.Frame

data class DeviceUiState(
    val isConnected: Boolean = false,
    val isConnecting: Boolean = true,

    val motion: Frame.Motion? = null,
    val env: Frame.Env? = null,

    val motionConnected: Boolean = true,
    val envConnected: Boolean = true,

    val error: String? = null
)