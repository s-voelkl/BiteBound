package com.example.bitebound.ui

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.Logout
import androidx.compose.material.icons.filled.PlayArrow
import androidx.compose.material.icons.filled.Stop
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.CenterAlignedTopAppBar
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.material3.TopAppBarDefaults
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import com.example.bitebound.UiState
import com.example.bitebound.data.Telemetry
import com.example.bitebound.mqtt.ConnectionState
import com.example.bitebound.ui.components.CookieCard
import com.example.bitebound.ui.components.CookieProgress
import com.example.bitebound.ui.components.MiniGameBoard
import com.example.bitebound.ui.components.StatGrid
import com.example.bitebound.ui.theme.BerryRed
import com.example.bitebound.ui.theme.ChocolateChip
import com.example.bitebound.ui.theme.Honey
import com.example.bitebound.ui.theme.MintGreen

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun DashboardScreen(
    state: UiState,
    onStart: (playerName: String, gameId: Int, cookiesCount: Int, wallThickness: Int) -> Unit,
    onStop: () -> Unit,
    onDisconnect: () -> Unit,
    modifier: Modifier = Modifier,
) {
    Scaffold(
        modifier = modifier,
        containerColor = MaterialTheme.colorScheme.background,
        topBar = {
            CenterAlignedTopAppBar(
                title = {
                    Row(verticalAlignment = Alignment.CenterVertically) {
                        Text("🍪", style = MaterialTheme.typography.titleLarge)
                        Spacer(Modifier.width(8.dp))
                        Text("BiteBound", style = MaterialTheme.typography.titleLarge)
                    }
                },
                actions = {
                    IconButton(onClick = onDisconnect) {
                        Icon(Icons.AutoMirrored.Filled.Logout, contentDescription = "Disconnect")
                    }
                },
                colors = TopAppBarDefaults.topAppBarColors(
                    containerColor = MaterialTheme.colorScheme.surface,
                    titleContentColor = MaterialTheme.colorScheme.primary,
                ),
            )
        },
    ) { innerPadding ->
        Column(
            Modifier
                .fillMaxSize()
                .padding(innerPadding)
                .verticalScroll(rememberScrollState())
                .padding(horizontal = 16.dp, vertical = 12.dp),
            verticalArrangement = Arrangement.spacedBy(14.dp),
        ) {
            ConnectionBanner(state.connection, state.messageCount, state.telemetry?.state?.runningStatus)

            val telemetry = state.telemetry
            if (telemetry == null) {
                WaitingForOven()
            } else {
                ScoreCard(telemetry)
            }

            ControlsCard(
                defaultPlayer = state.credentials.playerName,
                defaultCookies = state.credentials.cookiesCount,
                defaultGameId = state.credentials.gameId,
                defaultWall = state.credentials.wallThickness,
                connected = state.connection is ConnectionState.Connected,
                onStart = onStart,
                onStop = onStop,
            )

            if (telemetry != null) {
                BoardCard(telemetry)
                SensorsCard(telemetry)
                DeviceCard(telemetry)
            }
            Spacer(Modifier.height(8.dp))
        }
    }
}

@Composable
private fun ConnectionBanner(connection: ConnectionState, messageCount: Int, runningStatus: String?) {
    val (color, label) = when (connection) {
        ConnectionState.Connected -> {
            val status = runningStatus?.replaceFirstChar { it.uppercase() } ?: "Unknown"
            MintGreen to "$status · $messageCount updates"
        }
        ConnectionState.Connecting -> Honey to "Connecting…"
        is ConnectionState.Failed -> BerryRed to "Disconnected · ${connection.reason}"
        ConnectionState.Disconnected -> ChocolateChip to "Disconnected"
    }
    Surface(
        shape = RoundedCornerShape(14.dp),
        color = color.copy(alpha = 0.16f),
        modifier = Modifier.fillMaxWidth(),
    ) {
        Row(
            Modifier.padding(horizontal = 14.dp, vertical = 10.dp),
            verticalAlignment = Alignment.CenterVertically,
        ) {
            Box(
                Modifier
                    .size(10.dp)
                    .clip(RoundedCornerShape(50))
                    .background(color),
            )
            Spacer(Modifier.width(10.dp))
            Text(label, style = MaterialTheme.typography.labelLarge, color = color)
        }
    }
}

@Composable
private fun WaitingForOven() {
    CookieCard(title = "Warming up the oven", emoji = "⏳") {
        Text(
            "Connected — waiting for the first telemetry message from the ESP32.\n" +
                "Start the game below.",
            style = MaterialTheme.typography.bodyMedium,
            color = MaterialTheme.colorScheme.onSurfaceVariant,
        )
    }
}

@Composable
private fun ScoreCard(telemetry: Telemetry) {
    CookieCard(title = "Cookie Score", emoji = "🏆") {
        Box(Modifier.fillMaxWidth(), contentAlignment = Alignment.Center) {
            CookieProgress(
                collected = telemetry.state.cookiesCollected,
                target = telemetry.config.targetCookies,
            )
        }
        Spacer(Modifier.height(14.dp))
        Row(
            Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.SpaceEvenly,
        ) {
            HeroStat("Round", telemetry.state.currentRound.toString(), Honey)
            HeroStat("Time", formatDuration(telemetry.state.elapsedTimeSec), MintGreen)
        }
        Spacer(Modifier.height(10.dp))
        Text(
            "Player: ${telemetry.config.playerName}  ·  Remaining: ${telemetry.state.cookiesRemaining}",
            style = MaterialTheme.typography.bodyMedium,
            color = MaterialTheme.colorScheme.onSurfaceVariant,
            modifier = Modifier.fillMaxWidth(),
            textAlign = TextAlign.Center,
        )
    }
}

@Composable
private fun HeroStat(label: String, value: String, color: Color) {
    Column(horizontalAlignment = Alignment.CenterHorizontally) {
        Text(value, style = MaterialTheme.typography.titleLarge, color = color, fontWeight = FontWeight.Bold)
        Text(label, style = MaterialTheme.typography.labelSmall, color = MaterialTheme.colorScheme.onSurfaceVariant)
    }
}

@Composable
private fun ControlsCard(
    defaultPlayer: String,
    defaultCookies: Int,
    defaultGameId: Int,
    defaultWall: Int,
    connected: Boolean,
    onStart: (String, Int, Int, Int) -> Unit,
    onStop: () -> Unit,
) {
    var player by remember { mutableStateOf(defaultPlayer) }
    var cookies by remember { mutableStateOf(defaultCookies.toString()) }
    var gameId by remember { mutableStateOf(defaultGameId) }
    var wall by remember { mutableStateOf(defaultWall.toString()) }

    CookieCard(title = "Game Controls", emoji = "🎮") {
        OutlinedTextField(
            value = player,
            onValueChange = { player = it },
            label = { Text("Player name") },
            singleLine = true,
            shape = RoundedCornerShape(14.dp),
            modifier = Modifier.fillMaxWidth(),
        )
        Spacer(Modifier.height(10.dp))
        
        Row(Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            Button(
                onClick = { gameId = 1 },
                modifier = Modifier.weight(1f),
                colors = ButtonDefaults.buttonColors(
                    containerColor = if (gameId == 1) MaterialTheme.colorScheme.primary else MaterialTheme.colorScheme.surfaceVariant,
                    contentColor = if (gameId == 1) MaterialTheme.colorScheme.onPrimary else MaterialTheme.colorScheme.onSurfaceVariant
                ),
                shape = RoundedCornerShape(12.dp)
            ) {
                Text("Labyrinth")
            }
            Button(
                onClick = { gameId = 2 },
                modifier = Modifier.weight(1f),
                colors = ButtonDefaults.buttonColors(
                    containerColor = if (gameId == 2) MaterialTheme.colorScheme.primary else MaterialTheme.colorScheme.surfaceVariant,
                    contentColor = if (gameId == 2) MaterialTheme.colorScheme.onPrimary else MaterialTheme.colorScheme.onSurfaceVariant
                ),
                shape = RoundedCornerShape(12.dp)
            ) {
                Text("Flatland")
            }
        }
        
        Spacer(Modifier.height(10.dp))
        Row(Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            OutlinedTextField(
                value = cookies,
                onValueChange = { cookies = it.filter(Char::isDigit) },
                label = { Text("Cookies") },
                singleLine = true,
                keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Number),
                shape = RoundedCornerShape(14.dp),
                modifier = Modifier.weight(1f),
            )
            OutlinedTextField(
                value = wall,
                onValueChange = { wall = it.filter(Char::isDigit) },
                label = { Text("Wall Px") },
                singleLine = true,
                keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Number),
                shape = RoundedCornerShape(14.dp),
                modifier = Modifier.weight(1f),
            )
        }
        
        Spacer(Modifier.height(14.dp))
        Row(horizontalArrangement = Arrangement.spacedBy(12.dp)) {
            Button(
                onClick = { 
                    onStart(
                        player.trim().ifBlank { "Cookie-Lover" }, 
                        gameId,
                        cookies.toIntOrNull()?.coerceIn(1, 1000) ?: 10,
                        wall.toIntOrNull()?.coerceIn(5, 40) ?: 10
                    ) 
                },
                enabled = connected,
                modifier = Modifier.weight(1f).height(50.dp),
                shape = RoundedCornerShape(16.dp),
                colors = ButtonDefaults.buttonColors(containerColor = MintGreen, contentColor = Color.White),
            ) {
                Icon(Icons.Filled.PlayArrow, contentDescription = null)
                Spacer(Modifier.width(6.dp))
                Text("Start", fontWeight = FontWeight.Bold)
            }
            Button(
                onClick = { onStop() },
                enabled = connected,
                modifier = Modifier.weight(1f).height(50.dp),
                shape = RoundedCornerShape(16.dp),
                colors = ButtonDefaults.buttonColors(containerColor = BerryRed, contentColor = Color.White),
            ) {
                Icon(Icons.Filled.Stop, contentDescription = null)
                Spacer(Modifier.width(6.dp))
                Text("Stop", fontWeight = FontWeight.Bold)
            }
        }
    }
}

@Composable
private fun BoardCard(telemetry: Telemetry) {
    CookieCard(title = "Live Board", emoji = "🎯") {
        Box(Modifier.fillMaxWidth(), contentAlignment = Alignment.Center) {
            MiniGameBoard(
                ballX = telemetry.physics.ballPosX,
                ballY = telemetry.physics.ballPosY,
                velocityX = telemetry.physics.velocityX,
                velocityY = telemetry.physics.velocityY,
                screenWidth = telemetry.config.screenWidth,
                screenHeight = telemetry.config.screenHeight,
                collision = telemetry.physics.collisionDetected,
                modifier = Modifier.fillMaxWidth(0.8f)
            )
        }

        /*
        Spacer(Modifier.height(16.dp))
        
        StatGrid(
            stats = listOf(
                "Pos X" to fmt(telemetry.physics.ballPosX, 2),
                "Pos Y" to fmt(telemetry.physics.ballPosY, 2),
                "Collision" to if (telemetry.physics.collisionDetected) "Yes 💥" else "No",
                "Vel X" to fmt(telemetry.physics.velocityX, 2),
                "Vel Y" to fmt(telemetry.physics.velocityY, 2),
                "Accel X" to fmt(telemetry.physics.accX, 2),
            ),
        )
        */
    }
}

@Composable
private fun SensorsCard(telemetry: Telemetry) {
    CookieCard(title = "Sensors", emoji = "📡") {
        Row(
            Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.SpaceEvenly,
        ) {
            HeroStat(
                label = "Battery",
                value = "${fmt(telemetry.sensors.batteryVoltage, 2)}V",
                color = if (telemetry.sensors.batteryVoltage < 3.4) BerryRed else MintGreen
            )
            HeroStat(
                label = "Button",
                value = if (telemetry.sensors.button) "Pressed" else "Released",
                color = if (telemetry.sensors.button) Honey else Color.White
            )
        }

        Spacer(Modifier.height(16.dp))

        StatGrid(
            stats = listOf(
                "Accel X" to fmt(telemetry.sensors.accelX, 2),
                "Accel Y" to fmt(telemetry.sensors.accelY, 2),
                "Accel Z" to fmt(telemetry.sensors.accelZ, 2),
                "Gyro X" to fmt(telemetry.sensors.gyroX, 2),
                "Gyro Y" to fmt(telemetry.sensors.gyroY, 2),
                "Gyro Z" to fmt(telemetry.sensors.gyroZ, 2),
            ),
        )
    }
}

@Composable
private fun DeviceCard(telemetry: Telemetry) {
    CookieCard(title = "Device", emoji = "🔌") {
        InfoRow("Client ID", telemetry.device.clientId)
        InfoRow("Hardware", telemetry.device.hardware)
        InfoRow("Firmware", telemetry.device.firmwareVersion)
        InfoRow("Wi-Fi", telemetry.device.wifiSsid)
        InfoRow("Uptime", formatDuration(telemetry.device.uptimeMs / 1000.0))
    }
}

@Composable
private fun InfoRow(label: String, value: String) {
    Row(
        Modifier
            .fillMaxWidth()
            .padding(vertical = 4.dp),
        horizontalArrangement = Arrangement.SpaceBetween,
    ) {
        Text(label, style = MaterialTheme.typography.bodyMedium, color = MaterialTheme.colorScheme.onSurfaceVariant)
        Text(
            value,
            style = MaterialTheme.typography.bodyMedium,
            color = MaterialTheme.colorScheme.onSurface,
            fontWeight = FontWeight.Medium,
        )
    }
}

private fun runningStatusColor(runningStatus: String): Color = when (runningStatus.lowercase()) {
    "running" -> MintGreen
    "completed" -> Honey
    "stopped", "idle" -> ChocolateChip
    else -> ChocolateChip
}

private fun fmt(value: Double, decimals: Int): String = "%.${decimals}f".format(value)

private fun formatDuration(seconds: Double): String {
    val total = seconds.toInt()
    val m = total / 60
    val s = total % 60
    return if (m > 0) "%d:%02d".format(m, s) else "${s}s"
}
