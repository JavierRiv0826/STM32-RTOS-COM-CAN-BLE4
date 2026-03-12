package com.example.ble4app.navigation

object Routes {
    const val PERMISSION = "permission"
    const val SCANNER = "scanner"
    const val DEVICE = "device"
    fun deviceRoute(address: String) = "$DEVICE/$address"
}