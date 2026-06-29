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
import androidx.compose.material.icons.filled.Refresh
import androidx.compose.material.icons.filled.Stop
import androidx.compose.material3.AlertDialog
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
import androidx.compose.material3.TextButton
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
import com.example.bitebound.data.BallType
import com.example.bitebound.data.GameConfigConstants
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
    onStart: (playerName: String, gameId: Int, cookies: Int, wall: Int, restitution: Double) -> Unit,
    onStop: () -> Unit,
    onResume: (playerName: String) -> Unit,
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
                defaultRestitution = state.credentials.restitution,
                connected = state.connection is ConnectionState.Connected,
                running = telemetry?.state?.isRunning == true,
                hasTelemetry = telemetry != null,
                onStart = onStart,
                onStop = onStop,
                onResume = onResume,
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
        if (telemetry.state.isFinished) {
            Spacer(Modifier.height(12.dp))
            Text(
                "🎉 Round complete! Next round starting…",
                style = MaterialTheme.typography.titleMedium,
                color = MintGreen,
                fontWeight = FontWeight.Bold,
                modifier = Modifier.fillMaxWidth(),
                textAlign = TextAlign.Center,
            )
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
    defaultRestitution: Double,
    connected: Boolean,
    running: Boolean,
    hasTelemetry: Boolean,
    onStart: (String, Int, Int, Int, Double) -> Unit,
    onStop: () -> Unit,
    onResume: (String) -> Unit,
) {
    var player by remember { mutableStateOf(defaultPlayer) }
    var cookies by remember { mutableStateOf(defaultCookies.toString()) }
    var gameId by remember { mutableStateOf(defaultGameId) }
    var wall by remember { mutableStateOf(defaultWall.toString()) }
    var ballType by remember { mutableStateOf(BallType.fromRestitution(defaultRestitution)) }
    var showNewGameConfirm by remember { mutableStateOf(false) }

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
                Text("Baking Tray")
            }
        }

        Spacer(Modifier.height(10.dp))
        // Wall thickness only matters for the Labyrinth - the Baking Tray has no walls.
        Row(Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            OutlinedTextField(
                value = cookies,
                onValueChange = { cookies = clampCookieInput(it) },
                label = { Text("Cookies (1-20)") },
                singleLine = true,
                keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Number),
                shape = RoundedCornerShape(14.dp),
                modifier = Modifier.weight(1f),
            )
            if (gameId == 1) {
                OutlinedTextField(
                    value = wall,
                    onValueChange = { wall = clampWallInput(it) },
                    label = { Text("Wall Px (5-40)") },
                    singleLine = true,
                    keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Number),
                    shape = RoundedCornerShape(14.dp),
                    modifier = Modifier.weight(1f),
                )
            } else {
                Spacer(Modifier.weight(1f))
            }
        }
        
        Spacer(Modifier.height(10.dp))
        // Ball type picks the bounce (restitution) sent with the next start.
        Text(
            "Ball Type",
            style = MaterialTheme.typography.labelMedium,
            color = MaterialTheme.colorScheme.onSurfaceVariant,
            modifier = Modifier.padding(bottom = 4.dp),
        )
        Row(Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            BallType.entries.forEach { ball ->
                val selected = ballType == ball
                Button(
                    onClick = { ballType = ball },
                    modifier = Modifier.weight(1f),
                    colors = ButtonDefaults.buttonColors(
                        containerColor = if (selected) MaterialTheme.colorScheme.primary else MaterialTheme.colorScheme.surfaceVariant,
                        contentColor = if (selected) MaterialTheme.colorScheme.onPrimary else MaterialTheme.colorScheme.onSurfaceVariant
                    ),
                    shape = RoundedCornerShape(12.dp)
                ) {
                    Text(ball.label)
                }
            }
        }

        Spacer(Modifier.height(14.dp))
        Row(horizontalArrangement = Arrangement.spacedBy(12.dp)) {
            // Starts a fresh game - asks first because it throws away the current run.
            Button(
                onClick = { showNewGameConfirm = true },
                enabled = connected,
                modifier = Modifier.weight(1f).height(50.dp),
                shape = RoundedCornerShape(16.dp),
                colors = ButtonDefaults.buttonColors(containerColor = MintGreen, contentColor = Color.White),
            ) {
                Icon(Icons.Filled.Refresh, contentDescription = null)
                Spacer(Modifier.width(6.dp))
                Text("New Game", fontWeight = FontWeight.Bold)
            }
            // One button does both: pause a running game, resume a paused one.
            // Disabled until we have telemetry, otherwise it would misleadingly
            // show "Resume" while we're still waiting for the ESP.
            Button(
                onClick = {
                    if (running) onStop() else onResume(player.trim().ifBlank { "Player 1" })
                },
                enabled = connected && hasTelemetry,
                modifier = Modifier.weight(1f).height(50.dp),
                shape = RoundedCornerShape(16.dp),
                colors = ButtonDefaults.buttonColors(
                    containerColor = if (running) BerryRed else Honey,
                    contentColor = Color.White,
                ),
            ) {
                Icon(
                    if (running) Icons.Filled.Stop else Icons.Filled.PlayArrow,
                    contentDescription = null,
                )
                Spacer(Modifier.width(6.dp))
                Text(if (running) "Pause" else "Resume", fontWeight = FontWeight.Bold)
            }
        }
    }

    if (showNewGameConfirm) {
        AlertDialog(
            onDismissRequest = { showNewGameConfirm = false },
            title = { Text("Start a new game?") },
            text = { Text("This starts a fresh game and the current progress will be lost.") },
            confirmButton = {
                TextButton(
                    onClick = {
                        showNewGameConfirm = false
                        onStart(
                            player.trim().ifBlank { "Cookie-Lover" },
                            gameId,
                            cookies.toIntOrNull()?.coerceIn(1, 20) ?: 10,
                            wall.toIntOrNull()?.coerceIn(5, 40) ?: 10,
                            ballType.restitution
                        )
                    }
                ) { Text("New Game") }
            },
            dismissButton = {
                TextButton(onClick = { showNewGameConfirm = false }) { Text("Cancel") }
            },
        )
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
                modifier = Modifier.fillMaxWidth(0.8f)
            )
        }
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
                // Was Color.White before, which is invisible on the light card -
                // use a theme colour so "Released" is actually readable.
                value = if (telemetry.sensors.button) "Pressed" else "Released",
                color = if (telemetry.sensors.button) Honey else MaterialTheme.colorScheme.onSurface
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

// Keep only digits and clamp to the cookie max as you type, so e.g. 20000 turns
// into 20 right in the field. Empty stays empty so the box can be cleared;
// over-long numbers (too big for Int) are treated as over the max.
private fun clampCookieInput(raw: String): String {
    val digits = raw.filter(Char::isDigit)
    if (digits.isEmpty()) return ""
    val value = digits.toIntOrNull() ?: GameConfigConstants.MAX_COOKIES
    return value.coerceAtMost(GameConfigConstants.MAX_COOKIES).toString()
}

// Live max-clamp for wall thickness. Minimum (5) is left to the Start coerceIn
// so you can still type "40" without the first digit jumping to the minimum.
private fun clampWallInput(raw: String): String {
    val digits = raw.filter(Char::isDigit)
    if (digits.isEmpty()) return ""
    val value = digits.toIntOrNull() ?: GameConfigConstants.MAX_WALL_THICKNESS
    return value.coerceAtMost(GameConfigConstants.MAX_WALL_THICKNESS).toString()
}

private fun fmt(value: Double, decimals: Int): String = "%.${decimals}f".format(value)

private fun formatDuration(seconds: Double): String {
    val total = seconds.toInt()
    val m = total / 60
    val s = total % 60
    return if (m > 0) "%d:%02d".format(m, s) else "${s}s"
}
