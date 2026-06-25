package com.example.bitebound

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Scaffold
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.ui.Modifier
import androidx.lifecycle.viewmodel.compose.viewModel
import com.example.bitebound.mqtt.ConnectionState
import com.example.bitebound.ui.ConnectionScreen
import com.example.bitebound.ui.DashboardScreen
import com.example.bitebound.ui.theme.BiteBoundTheme

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContent {
            BiteBoundTheme {
                BiteBoundApp()
            }
        }
    }
}

@Composable
private fun BiteBoundApp(viewModel: BiteBoundViewModel = viewModel()) {
    val state by viewModel.uiState.collectAsState()

    // Stay on the connection screen until the broker confirms us as connected;
    // a dropped connection (terminal failure) falls back here too.
    when (state.connection) {
        ConnectionState.Connected -> {
            DashboardScreen(
                state = state,
                onStart = viewModel::startGame,
                onStop = viewModel::stopGame,
                onDisconnect = viewModel::disconnect,
                modifier = Modifier.fillMaxSize(),
            )
        }

        else -> {
            Scaffold(modifier = Modifier.fillMaxSize()) { innerPadding ->
                ConnectionScreen(
                    credentials = state.credentials,
                    connection = state.connection,
                    onConnect = viewModel::connect,
                    modifier = Modifier
                        .fillMaxSize()
                        .padding(innerPadding),
                )
            }
        }
    }
}
