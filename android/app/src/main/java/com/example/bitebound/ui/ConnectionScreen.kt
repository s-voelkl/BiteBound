package com.example.bitebound.ui

import androidx.compose.animation.AnimatedVisibility
import androidx.compose.foundation.Image
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
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.text.input.PasswordVisualTransformation
import androidx.compose.ui.text.input.VisualTransformation
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import com.example.bitebound.data.BallType
import com.example.bitebound.R
import com.example.bitebound.data.Credentials
import com.example.bitebound.data.GameConfigConstants
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
    var ballType by remember { mutableStateOf(BallType.fromRestitution(credentials.restitution)) }
    var showPassword by remember { mutableStateOf(false) }
    var showServerConnetion by remember { mutableStateOf(false)}
    var showTopics by remember { mutableStateOf(false) }

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
            Image(
                painter = painterResource(id = R.drawable.logo),
                contentDescription = null,
                modifier = Modifier.size(100.dp),
            )
            Spacer(Modifier.height(16.dp))
            Text(
                "BiteBound",
                style = MaterialTheme.typography.headlineLarge,
                color = MaterialTheme.colorScheme.primary,
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
                    Text("Baking Tray")
                }
            }

            // The Baking Tray is just an open field with no maze, so wall
            // thickness has no effect there - only show it for the Labyrinth.
            Row(Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                CredentialField(
                    value = cookiesCount,
                    onValueChange = { cookiesCount = clampCookieInput(it) },
                    label = "Cookies (1-20)",
                    keyboardType = KeyboardType.Number,
                    enabled = !connecting,
                    modifier = Modifier.weight(1f)
                )
                if (gameId == 1) {
                    CredentialField(
                        value = wallThickness,
                        onValueChange = { wallThickness = clampWallInput(it) },
                        label = "Wall Px (5-20)",
                        keyboardType = KeyboardType.Number,
                        enabled = !connecting,
                        modifier = Modifier.weight(1f)
                    )
                }
            }

            // Ball type maps to a fixed restitution (bounce) value below.
            Text(
                "Ball Type",
                style = MaterialTheme.typography.labelMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant,
                modifier = Modifier.fillMaxWidth().padding(top = 6.dp, bottom = 4.dp),
            )
            Row(Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                BallType.entries.forEach { ball ->
                    val selected = ballType == ball
                    Button(
                        onClick = { ballType = ball },
                        enabled = !connecting,
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

            Spacer(Modifier.height(16.dp))
            // Server Connection Section
            TextButton(
                onClick = { showServerConnetion = !showServerConnetion },
                modifier = Modifier.align(Alignment.Start),
            ) {
                Icon(
                    if (showServerConnetion) Icons.Filled.ExpandLess else Icons.Filled.ExpandMore,
                    contentDescription = null,
                )
                Spacer(Modifier.width(4.dp))
                Text("Server Connection")
            }
            AnimatedVisibility(visible = showServerConnetion) {
                Column {

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
                }
            }

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

            // List whatever the button is still waiting on, so a greyed-out
            // button is never a mystery.
            val missing = buildList {
                if (playerName.isBlank()) add("Player Name")
                if (host.isBlank()) add("Broker Host Address")
                if (parsedPort !in 1..65535) add("Port (1-65535)")
                if (username.isBlank()) add("Username")
                if (password.isBlank()) add("Password")
            }
            val canConnect = missing.isEmpty() && !connecting

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
                            cookiesCount = parsedCookies.coerceIn(1, 20),
                            wallThickness = parsedWall.coerceIn(5, 40),
                            imuSensitivity = ballType.sensitivity,
                            restitution = ballType.restitution,
                            emaAlpha = ballType.emaAlpha,
                            deadzone = credentials.deadzone
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

            if (!connecting && missing.isNotEmpty()) {
                Spacer(Modifier.height(8.dp))
                Text(
                    "Still needed: ${missing.joinToString(", ")}",
                    style = MaterialTheme.typography.labelSmall,
                    color = BerryRed,
                    textAlign = TextAlign.Center,
                )
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

// Keep only digits and clamp straight away to the cookie max, so typing 20000
// snaps to 20 in the field itself instead of waiting for Start. Empty stays
// empty so the box can still be cleared and retyped. toIntOrNull catches numbers
// too long to fit an Int and just treats them as "way over the max".
private fun clampCookieInput(raw: String): String {
    val digits = raw.filter(Char::isDigit)
    if (digits.isEmpty()) return ""
    val value = digits.toIntOrNull() ?: GameConfigConstants.MAX_COOKIES
    return value.coerceAtMost(GameConfigConstants.MAX_COOKIES).toString()
}

// Same idea for wall thickness: clamp the max as you type. We only clamp the top
// end here - the minimum (5) is enforced on Start, otherwise you couldn't type
// "40" because the first "4" would jump up to the minimum.
private fun clampWallInput(raw: String): String {
    val digits = raw.filter(Char::isDigit)
    if (digits.isEmpty()) return ""
    val value = digits.toIntOrNull() ?: GameConfigConstants.MAX_WALL_THICKNESS
    return value.coerceAtMost(GameConfigConstants.MAX_WALL_THICKNESS).toString()
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
