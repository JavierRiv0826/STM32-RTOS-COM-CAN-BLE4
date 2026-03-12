package com.example.ble4app.presentation.device

import android.util.Log
import androidx.lifecycle.SavedStateHandle
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.example.ble4app.data.protocol.Frame
import com.example.ble4app.data.repository.BleRepository
import dagger.hilt.android.lifecycle.HiltViewModel
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch
import javax.inject.Inject

@HiltViewModel
class DeviceViewModel @Inject constructor(
    private val repository: BleRepository,
    savedStateHandle: SavedStateHandle
) : ViewModel() {

    private val address: String =
        savedStateHandle["address"] ?: ""

    private val _uiState = MutableStateFlow(DeviceUiState())
    val uiState: StateFlow<DeviceUiState> =
        _uiState.asStateFlow()

    init {
        connect()
        observeConnection()
        observeNotifications()
        observeFrames()
    }

    private fun connect() {
        viewModelScope.launch {
            try {
                repository.connect(address)
            } catch (e: Exception) {
                _uiState.update {
                    it.copy(
                        isConnecting = false,
                        error = e.message
                    )
                }
            }
        }
    }

    private fun observeConnection() {
        viewModelScope.launch {
            repository.observeConnectionState()
                .collect { connected ->
                    _uiState.update {
                        it.copy(
                            isConnected = connected,
                            isConnecting = false
                        )
                    }
                }
        }
    }

    private fun observeNotifications() {
        viewModelScope.launch {
            repository.observeNotifications()
                .collect { data ->
                    Log.d("DB_BLE", "Data: ${data.joinToString()}")
                }
        }
    }

    private fun observeFrames() {
        viewModelScope.launch {

            repository.observeFrames()
                .collect { frame ->

                    _uiState.update { current ->

                        when (frame) {

                            is Frame.Motion ->
                                current.copy(
                                    motion = frame,
                                    motionConnected = true
                                )

                            is Frame.Env ->
                                current.copy(
                                    env = frame,
                                    envConnected = true
                                )

                            is Frame.Status -> {

                                when {
                                    frame.statusType == 0x01 &&
                                            frame.nodeType == 0x01 -> {
                                        // Motion disconnected
                                        current.copy(
                                            motionConnected = false,
                                            motion = null
                                        )
                                    }

                                    frame.statusType == 0x01 &&
                                            frame.nodeType == 0x02 -> {
                                        // Env disconnected
                                        current.copy(
                                            envConnected = false,
                                            env = null
                                        )
                                    }

                                    else -> current
                                }
                            }
                        }
                    }
                }
        }
    }

    fun sendCommand(command: Int) {
        viewModelScope.launch {
            repository.sendCommand(command)
        }
    }

    override fun onCleared() {
        viewModelScope.launch {
            repository.disconnect()
        }
        super.onCleared()
    }
}