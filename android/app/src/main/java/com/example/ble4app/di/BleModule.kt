package com.example.ble4app.di

import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothManager
import android.content.Context
import com.example.ble4app.data.ble.BleDataSource
import com.example.ble4app.data.ble.BleDataSourceImpl
import dagger.Module
import dagger.Provides
import dagger.hilt.InstallIn
import dagger.hilt.android.qualifiers.ApplicationContext
import dagger.hilt.components.SingletonComponent
import javax.inject.Singleton

@Module
@InstallIn(SingletonComponent::class)
object BleModule {

    @Provides
    @Singleton
    fun provideBluetoothAdapter(
        @ApplicationContext context: Context
    ): BluetoothAdapter {
        val manager =
            context.getSystemService(Context.BLUETOOTH_SERVICE)
                    as BluetoothManager
        return manager.adapter
    }

    @Provides
    @Singleton
    fun provideBleDataSource(
        @ApplicationContext context: Context,
        bluetoothAdapter: BluetoothAdapter
    ): BleDataSource {
        return BleDataSourceImpl(
            context = context,
            bluetoothAdapter = bluetoothAdapter
        )
    }
}