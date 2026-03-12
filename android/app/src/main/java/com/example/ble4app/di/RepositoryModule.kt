package com.example.ble4app.di

import com.example.ble4app.data.repository.BleRepository
import com.example.ble4app.data.repository.BleRepositoryImpl
import dagger.Binds
import dagger.Module
import dagger.hilt.InstallIn
import dagger.hilt.components.SingletonComponent
import javax.inject.Singleton

@Module
@InstallIn(SingletonComponent::class)
abstract class RepositoryModule {

    @Binds
    @Singleton
    abstract fun bindBleRepository(
        impl: BleRepositoryImpl
    ): BleRepository
}