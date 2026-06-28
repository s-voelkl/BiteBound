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
import com.example.bitebound.R
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
    var telemetryTopic by remember { mutableStateOf(credentials.telemetryTopic) }
    var commandTopic by remember { mutableStateOf(credentials.commandTopic) }
    var showPassword by remember { mutableStateOf(false) }
    var showAdvanced by remember { mutableStateOf(false) }

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
            Text(
                "Connect to your cookie game",
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant,
            )
            Spacer(Modifier.height(28.dp))

            CredentialField(
                value = host,
                onValueChange = { host = it },
                label = "Broker host",
                enabled = !connecting,
            )
            CredentialField(
                value = port,
                onValueChange = { port = it.filter(Char::isDigit) },
                label = "Port",
                keyboardType = KeyboardType.Number,
                enabled = !connecting,
            )
            CredentialField(
                value = username,
                onValueChange = { username = it },
                label = "Username",
                enabled = !connecting,
            )
            CredentialField(
                value = password,
                onValueChange = { password = it },
                label = "Password",
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
                onClick = { showAdvanced = !showAdvanced },
                modifier = Modifier.align(Alignment.Start),
            ) {
                Icon(
                    if (showAdvanced) Icons.Filled.ExpandLess else Icons.Filled.ExpandMore,
                    contentDescription = null,
                )
                Spacer(Modifier.width(4.dp))
                Text("Topics & advanced")
            }
            AnimatedVisibility(visible = showAdvanced) {
                Column {
                    CredentialField(
                        value = telemetryTopic,
                        onValueChange = { telemetryTopic = it },
                        label = "Telemetry topic (subscribe)",
                        enabled = !connecting,
                    )
                    CredentialField(
                        value = commandTopic,
                        onValueChange = { commandTopic = it },
                        label = "Command topic (publish)",
                        enabled = !connecting,
                    )
                }
            }

            Spacer(Modifier.height(8.dp))

            val parsedPort = port.toIntOrNull() ?: -1
            val canConnect = host.isNotBlank() && parsedPort in 1..65535 &&
                username.isNotBlank() && password.isNotBlank() && !connecting

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
        modifier = Modifier
            .fillMaxWidth()
            .padding(vertical = 6.dp),
    )
}
