package com.example.bitebound.ui

import androidx.compose.animation.AnimatedVisibility
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ExpandLess
import androidx.compose.material.icons.filled.ExpandMore
import androidx.compose.material.icons.filled.Visibility
import androidx.compose.material.icons.filled.VisibilityOff
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.text.input.PasswordVisualTransformation
import androidx.compose.ui.text.input.VisualTransformation
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import com.example.bitebound.data.Credentials
import com.example.bitebound.mqtt.ConnectionState
import com.example.bitebound.ui.theme.BerryRed
import com.example.bitebound.ui.theme.ChocolateChip

@Composable
fun ConnectionScreen(
    credentials: Credentials,
    connection: ConnectionState,
    onConnect: (Credentials) -> Unit,
    modifier: Modifier = Modifier,
) {
    var host by remember { mutableStateOf(credentials.host) }
    var port by remember { mutableStateOf(credentials.port.toString()) }
    var username by remember { mutableStateOf(credentials.username) }
    var password by remember { mutableStateOf(credentials.password) }
    var playerName by remember { mutableStateOf(credentials.playerName) }
    var gameId by remember { mutableStateOf(credentials.gameId) }
    var cookiesCount by remember { mutableStateOf(credentials.cookiesCount.toString()) }
    var wallThickness by remember { mutableStateOf(credentials.wallThickness.toString()) }
    var telemetryTopic by remember { mutableStateOf(credentials.telemetryTopic) }
    var commandTopic by remember { mutableStateOf(credentials.commandTopic) }
    var imuSensitivity by remember { mutableStateOf(credentials.imuSensitivity.toString()) }
    var restitution by remember { mutableStateOf(credentials.restitution.toString()) }
    var emaAlpha by remember { mutableStateOf(credentials.emaAlpha.toString()) }
    var deadzone by remember { mutableStateOf(credentials.deadzone.toString()) }
    var showPassword by remember { mutableStateOf(false) }
    var showTopics by remember { mutableStateOf(false) }
    var showPhysics by remember { mutableStateOf(false) }

    val connecting = connection is ConnectionState.Connecting

    Box(
        modifier
            .fillMaxSize()
            .padding(horizontal = 24.dp)
            .verticalScroll(rememberScrollState()),
        contentAlignment = Alignment.TopCenter,
    ) {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .padding(vertical = 32.dp),
            horizontalAlignment = Alignment.CenterHorizontally,
        ) {
            Text("🍪", style = MaterialTheme.typography.headlineLarge.copy(fontSize = MaterialTheme.typography.headlineLarge.fontSize * 2))
            Spacer(Modifier.height(8.dp))
            Text(
                "BiteBound",
                style = MaterialTheme.typography.headlineLarge,
                color = MaterialTheme.colorScheme.primary,
            )
            Text(
                "The sweetest connection in IoT",
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant,
            )
            Spacer(Modifier.height(28.dp))

            // Game Settings Section
            Text(
                "Game Settings",
                style = MaterialTheme.typography.titleMedium,
                modifier = Modifier.fillMaxWidth().padding(bottom = 8.dp),
                fontWeight = FontWeight.Bold
            )

            CredentialField(
                value = playerName,
                onValueChange = { playerName = it },
                label = "Player Name",
                enabled = !connecting,
            )
            
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

            Row(Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                CredentialField(
                    value = cookiesCount,
                    onValueChange = { cookiesCount = it.filter(Char::isDigit) },
                    label = "Cookies (1-1000)",
                    keyboardType = KeyboardType.Number,
                    enabled = !connecting,
                    modifier = Modifier.weight(1f)
                )
                CredentialField(
                    value = wallThickness,
                    onValueChange = { wallThickness = it.filter(Char::isDigit) },
                    label = "Wall Px (5-40)",
                    keyboardType = KeyboardType.Number,
                    enabled = !connecting,
                    modifier = Modifier.weight(1f)
                )
            }

            TextButton(
                onClick = { showPhysics = !showPhysics },
                modifier = Modifier.align(Alignment.Start),
            ) {
                Icon(
                    if (showPhysics) Icons.Filled.ExpandLess else Icons.Filled.ExpandMore,
                    contentDescription = null,
                )
                Spacer(Modifier.width(4.dp))
                Text("Physics Parameters")
            }
            AnimatedVisibility(visible = showPhysics) {
                Column {
                    Row(Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                        CredentialField(
                            value = imuSensitivity,
                            onValueChange = { imuSensitivity = it },
                            label = "Sensitivity",
                            keyboardType = KeyboardType.Decimal,
                            enabled = !connecting,
                            modifier = Modifier.weight(1f)
                        )
                        CredentialField(
                            value = restitution,
                            onValueChange = { restitution = it },
                            label = "Restitution",
                            keyboardType = KeyboardType.Decimal,
                            enabled = !connecting,
                            modifier = Modifier.weight(1f)
                        )
                    }
                    Row(Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                        CredentialField(
                            value = emaAlpha,
                            onValueChange = { emaAlpha = it },
                            label = "EMA Alpha",
                            keyboardType = KeyboardType.Decimal,
                            enabled = !connecting,
                            modifier = Modifier.weight(1f)
                        )
                        CredentialField(
                            value = deadzone,
                            onValueChange = { deadzone = it },
                            label = "Deadzone",
                            keyboardType = KeyboardType.Decimal,
                            enabled = !connecting,
                            modifier = Modifier.weight(1f)
                        )
                    }
                }
            }

            Spacer(Modifier.height(16.dp))
            // Server Connection Section
            Text(
                "Server Connection",
                style = MaterialTheme.typography.titleMedium,
                modifier = Modifier.fillMaxWidth().padding(bottom = 8.dp),
                fontWeight = FontWeight.Bold
            )

            CredentialField(
                value = host,
                onValueChange = { host = it },
                label = "Broker Host Address",
                enabled = !connecting,
            )
            CredentialField(
                value = port,
                onValueChange = { port = it.filter(Char::isDigit) },
                label = "Port (default 8883)",
                keyboardType = KeyboardType.Number,
                enabled = !connecting,
            )
            CredentialField(
                value = username,
                onValueChange = { username = it },
                label = "Username for Connection",
                enabled = !connecting,
            )
            CredentialField(
                value = password,
                onValueChange = { password = it },
                label = "Password for Connection",
                enabled = !connecting,
                keyboardType = KeyboardType.Password,
                visualTransformation = if (showPassword) {
                    VisualTransformation.None
                } else {
                    PasswordVisualTransformation()
                },
                trailingIcon = {
                    IconButton(onClick = { showPassword = !showPassword }) {
                        Icon(
                            if (showPassword) Icons.Filled.VisibilityOff else Icons.Filled.Visibility,
                            contentDescription = if (showPassword) "Hide password" else "Show password",
                        )
                    }
                },
            )

            TextButton(
                onClick = { showTopics = !showTopics },
                modifier = Modifier.align(Alignment.Start),
            ) {
                Icon(
                    if (showTopics) Icons.Filled.ExpandLess else Icons.Filled.ExpandMore,
                    contentDescription = null,
                )
                Spacer(Modifier.width(4.dp))
                Text("Connection Topics")
            }
            AnimatedVisibility(visible = showTopics) {
                Column {
                    CredentialField(
                        value = telemetryTopic,
                        onValueChange = { telemetryTopic = it },
                        label = "Telemetry topic",
                        enabled = !connecting,
                    )
                    CredentialField(
                        value = commandTopic,
                        onValueChange = { commandTopic = it },
                        label = "Command topic",
                        enabled = !connecting,
                    )
                }
            }

            Spacer(Modifier.height(16.dp))

            val parsedPort = port.toIntOrNull() ?: -1
            val parsedCookies = cookiesCount.toIntOrNull() ?: 10
            val parsedWall = wallThickness.toIntOrNull() ?: 10
            
            val canConnect = host.isNotBlank() && parsedPort in 1..65535 &&
                username.isNotBlank() && password.isNotBlank() && 
                playerName.isNotBlank() && !connecting

            Button(
                onClick = {
                    onConnect(
                        Credentials(
                            host = host.trim(),
                            port = parsedPort,
                            username = username.trim(),
                            password = password,
                            telemetryTopic = telemetryTopic.trim(),
                            commandTopic = commandTopic.trim(),
                            playerName = playerName.trim(),
                            gameId = gameId,
                            cookiesCount = parsedCookies.coerceIn(1, 1000),
                            wallThickness = parsedWall.coerceIn(5, 40),
                            imuSensitivity = imuSensitivity.toDoubleOrNull() ?: credentials.imuSensitivity,
                            restitution = restitution.toDoubleOrNull() ?: credentials.restitution,
                            emaAlpha = emaAlpha.toDoubleOrNull() ?: credentials.emaAlpha,
                            deadzone = deadzone.toDoubleOrNull() ?: credentials.deadzone
                        ),
                    )
                },
                enabled = canConnect,
                modifier = Modifier
                    .fillMaxWidth()
                    .height(54.dp),
                shape = RoundedCornerShape(18.dp),
                colors = ButtonDefaults.buttonColors(
                    containerColor = MaterialTheme.colorScheme.primary,
                    contentColor = MaterialTheme.colorScheme.onPrimary,
                ),
            ) {
                if (connecting) {
                    CircularProgressIndicator(
                        modifier = Modifier.height(22.dp).width(22.dp),
                        color = MaterialTheme.colorScheme.onPrimary,
                        strokeWidth = 2.5.dp,
                    )
                    Spacer(Modifier.width(12.dp))
                    Text("Connecting…", fontWeight = FontWeight.Bold)
                } else {
                    Text("Bake the connection 🍪", fontWeight = FontWeight.Bold)
                }
            }

            if (connection is ConnectionState.Failed) {
                Spacer(Modifier.height(16.dp))
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Text("🔥", style = MaterialTheme.typography.titleMedium)
                    Spacer(Modifier.width(8.dp))
                    Text(
                        "Burnt batch: ${connection.reason}",
                        style = MaterialTheme.typography.bodyMedium,
                        color = BerryRed,
                        textAlign = TextAlign.Start,
                    )
                }
            }

            Spacer(Modifier.height(24.dp))
            Text(
                "Credentials are stored on this device only.",
                style = MaterialTheme.typography.labelSmall,
                color = ChocolateChip.copy(alpha = 0.6f),
                textAlign = TextAlign.Center,
            )
        }
    }
}

@Composable
private fun CredentialField(
    value: String,
    onValueChange: (String) -> Unit,
    label: String,
    enabled: Boolean = true,
    keyboardType: KeyboardType = KeyboardType.Text,
    visualTransformation: VisualTransformation = VisualTransformation.None,
    trailingIcon: @Composable (() -> Unit)? = null,
    modifier: Modifier = Modifier,
) {
    OutlinedTextField(
        value = value,
        onValueChange = onValueChange,
        label = { Text(label) },
        singleLine = true,
        enabled = enabled,
        keyboardOptions = KeyboardOptions(keyboardType = keyboardType),
        visualTransformation = visualTransformation,
        trailingIcon = trailingIcon,
        shape = RoundedCornerShape(14.dp),
        modifier = modifier
            .fillMaxWidth()
            .padding(vertical = 6.dp),
    )
}
