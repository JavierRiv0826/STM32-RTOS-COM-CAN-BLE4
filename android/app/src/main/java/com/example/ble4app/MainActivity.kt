package com.example.ble4app

import android.Manifest
import android.app.Activity
import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.core.content.ContextCompat
import androidx.navigation.NavType
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.rememberNavController
import androidx.navigation.navArgument
import com.example.ble4app.navigation.Routes
import com.example.ble4app.presentation.device.DeviceScreen
import com.example.ble4app.presentation.scanner.ScannerScreen
import com.example.ble4app.ui.theme.Ble4appTheme
import dagger.hilt.android.AndroidEntryPoint

@AndroidEntryPoint
class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContent {
            Ble4appTheme {
                BleAppNavHost()
            }
        }
    }
}

@Composable
fun BleAppNavHost() {

    val navController = rememberNavController()

    NavHost(
        navController = navController,
        startDestination = Routes.PERMISSION
    ) {

        composable(Routes.PERMISSION) {
            PermissionScreen(
                onPermissionsGranted = {
                    navController.navigate(Routes.SCANNER) {
                        popUpTo(Routes.PERMISSION) { inclusive = true }
                    }
                }
            )
        }

        composable(Routes.SCANNER) {
            ScannerScreen(navController)
        }

        composable(
            route = "${Routes.DEVICE}/{address}",
            arguments = listOf(
                navArgument("address") { type = NavType.StringType }
            )
        ) { backStackEntry ->

            val address =
                backStackEntry.arguments?.getString("address") ?: ""

            DeviceScreen(
                address = address
            )
        }
    }
}

@Composable
fun PermissionScreen(
    onPermissionsGranted: () -> Unit
) {

    val context = LocalContext.current
    val activity = context as Activity

    val permissions = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
        arrayOf(
            Manifest.permission.BLUETOOTH_SCAN,
            Manifest.permission.BLUETOOTH_CONNECT
        )
    } else {
        arrayOf(
            Manifest.permission.ACCESS_FINE_LOCATION
        )
    }

    val launcher = rememberLauncherForActivityResult(
        contract = ActivityResultContracts.RequestMultiplePermissions()
    ) { result ->

        val granted = result.values.all { it }

        if (granted) {
            onPermissionsGranted()
        }
    }

    LaunchedEffect(Unit) {

        val alreadyGranted = permissions.all {
            ContextCompat.checkSelfPermission(
                context,
                it
            ) == PackageManager.PERMISSION_GRANTED
        }

        if (alreadyGranted) {
            onPermissionsGranted()
        } else {
            launcher.launch(permissions)
        }
    }

    Box(
        modifier = Modifier.fillMaxSize(),
        contentAlignment = Alignment.Center
    ) {
        Text("Requesting BLE permissions...")
    }
}