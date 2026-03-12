package com.example.ble4app.presentation.scanner

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.example.ble4app.data.ble.BleDataSource
import dagger.hilt.android.lifecycle.HiltViewModel
import kotlinx.coroutines.Job
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.catch
import kotlinx.coroutines.flow.distinctUntilChangedBy
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch
import javax.inject.Inject

@HiltViewModel
class ScannerViewModel @Inject constructor(
    private val bleDataSource: BleDataSource
) : ViewModel() {

    private val _uiState = MutableStateFlow(ScannerUiState())
    val uiState: StateFlow<ScannerUiState> = _uiState.asStateFlow()

    private var scanJob: Job? = null

    fun startScan() {

        if (scanJob != null) return

        _uiState.update {
            it.copy(
                isScanning = true,
                devices = emptyList(),
                error = null
            )
        }

        scanJob = viewModelScope.launch {

            bleDataSource.scan()
                .distinctUntilChangedBy { it.address }
                .catch { e ->
                    _uiState.update {
                        it.copy(
                            isScanning = false,
                            error = e.message
                        )
                    }
                }
                .collect { device ->

                    _uiState.update { currentState ->

                        val alreadyExists =
                            currentState.devices.any { it.address == device.address }

                        if (alreadyExists) {
                            currentState
                        } else {
                            currentState.copy(
                                devices = currentState.devices + device
                            )
                        }
                    }
                }
        }
    }

    fun stopScan() {
        scanJob?.cancel()
        scanJob = null

        _uiState.update {
            it.copy(isScanning = false)
        }
    }

    override fun onCleared() {
        stopScan()
        super.onCleared()
    }
}