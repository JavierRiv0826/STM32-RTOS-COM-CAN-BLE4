package com.example.ble4app.presentation.device

import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Button
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp
import androidx.hilt.lifecycle.viewmodel.compose.hiltViewModel
import com.example.ble4app.data.protocol.Commands

@Composable
fun DeviceScreen(
    address: String,
    viewModel: DeviceViewModel = hiltViewModel()
) {

    val state by viewModel.uiState.collectAsState()

    Column(
        modifier = Modifier
            .fillMaxSize()
            .padding(16.dp)
    ) {

        Text("Device: $address")

        Spacer(modifier = Modifier.height(16.dp))

        when {
            state.isConnecting ->
                Text("Connecting...")

            !state.isConnected ->
                Text("Disconnected")

            else -> {

                Text("✅ Connected")

                Spacer(modifier = Modifier.height(16.dp))

                Text("Env Node")

                if (!state.envConnected) {
                    Text(
                        text = "DISCONNECTED",
                        color = Color.Red
                    )
                } else {
                    state.env?.let {
                        Text("BMP Temp: ${it.bmpTemp} °C")
                        Text("AHT Temp: ${it.ahtTemp} °C")
                        Text("Humidity: ${it.humidity} %")
                        Text("Pressure: ${it.pressure} hPa")
                        Text("Altitude: ${it.altitude} m")
                    }
                }

                Spacer(modifier = Modifier.height(12.dp))

                Text("Motion Node")

                if (!state.motionConnected) {
                    Text(
                        text = "DISCONNECTED",
                        color = Color.Red
                    )
                } else {
                    state.motion?.let {
                        Text("Node ID: ${it.nodeId}")
                        Text("Timestamp: ${it.timestamp}")
                    }
                }

                Spacer(modifier = Modifier.height(24.dp))

                Button(onClick = { viewModel.sendCommand(Commands.ALL_LED_ON) }) {
                    Text("All LED ON")
                }

                Button(onClick = { viewModel.sendCommand(Commands.ALL_LED_OFF) }) {
                    Text("All LED OFF")
                }

                Button(onClick = { viewModel.sendCommand(Commands.ENV_LED_ON) }) {
                    Text("Env LED ON")
                }

                Button(onClick = { viewModel.sendCommand(Commands.ENV_LED_OFF) }) {
                    Text("Env LED OFF")
                }

                Button(onClick = { viewModel.sendCommand(Commands.MOT_LED_ON) }) {
                    Text("Motion LED ON")
                }

                Button(onClick = { viewModel.sendCommand(Commands.MOT_LED_OFF) }) {
                    Text("Motion LED OFF")
                }
            }
        }

        state.error?.let {
            Spacer(modifier = Modifier.height(12.dp))
            Text("Error: $it")
        }
    }
}