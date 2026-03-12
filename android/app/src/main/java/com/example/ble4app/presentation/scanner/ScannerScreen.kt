package com.example.ble4app.presentation.scanner

import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material3.Button
import androidx.compose.material3.Card
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import androidx.hilt.lifecycle.viewmodel.compose.hiltViewModel
import androidx.navigation.NavController
import com.example.ble4app.data.ble.BleDevice
import com.example.ble4app.navigation.Routes

@Composable
fun ScannerScreen(
    navController: NavController,
    viewModel: ScannerViewModel = hiltViewModel()
) {

    val state by viewModel.uiState.collectAsState()

    Column(
        modifier = Modifier
            .fillMaxSize()
            .padding(16.dp)
    ) {

        Button(
            onClick = {
                if (state.isScanning)
                    viewModel.stopScan()
                else
                    viewModel.startScan()
            }
        ) {
            Text(if (state.isScanning) "Stop Scan" else "Scan")
        }

        Spacer(modifier = Modifier.height(16.dp))

        LazyColumn {
            items(state.devices) { device ->

                DeviceItem(
                    device = device,
                    onClick = {
                        viewModel.stopScan()

                        navController.navigate(
                            Routes.deviceRoute(it.address)
                        )
                    }
                )
            }
        }
    }
}

@Composable
fun DeviceItem(
    device: BleDevice,
    onClick: (BleDevice) -> Unit
) {

    Card(
        modifier = Modifier
            .fillMaxWidth()
            .padding(vertical = 4.dp)
            .clickable { onClick(device) }
    ) {
        Column(
            modifier = Modifier.padding(12.dp)
        ) {
            Text(
                text = device.name ?: "Unknown Device",
                style = MaterialTheme.typography.titleMedium
            )
            Text(
                text = device.address,
                style = MaterialTheme.typography.bodySmall
            )
        }
    }
}