package com.example.bitebound

import android.app.Application
import android.util.Log
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.viewModelScope
import com.example.bitebound.data.Credentials
import com.example.bitebound.data.CredentialsStore
import com.example.bitebound.data.GameCommand
import com.example.bitebound.data.GameConfigConstants
import com.example.bitebound.data.Telemetry
import com.example.bitebound.mqtt.ConnectionState
import com.example.bitebound.mqtt.MqttManager
import kotlinx.coroutines.Job
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.launch

/** One snapshot of everything the screens need to draw. */
data class UiState(
    val credentials: Credentials = Credentials(),
    val connection: ConnectionState = ConnectionState.Disconnected,
    val telemetry: Telemetry? = null,
    val messageCount: Int = 0,
)

class BiteBoundViewModel(app: Application) : AndroidViewModel(app) {

    private val store = CredentialsStore(app)
    private val mqtt = MqttManager()
    private var timeoutJob: Job? = null

    private val _uiState = MutableStateFlow(UiState(credentials = store.load()))
    val uiState: StateFlow<UiState> = _uiState.asStateFlow()

    /** Saves the credentials and opens the MQTT connection. */
    fun connect(credentials: Credentials) {
        store.save(credentials)
        _uiState.value = _uiState.value.copy(
            credentials = credentials,
            connection = ConnectionState.Connecting,
        )
        
        viewModelScope.launch {
            try {
                mqtt.connect(
                    credentials = credentials,
                    onMessage = { raw ->
                        try {
                            Telemetry.parse(raw)?.let { telemetry ->
                                _uiState.value = _uiState.value.copy(
                                    telemetry = telemetry,
                                    messageCount = _uiState.value.messageCount + 1,
                                )
                                resetTimeout()
                            }
                        } catch (e: Exception) {
                            Log.e("BiteBoundViewModel", "Error parsing telemetry", e)
                        }
                    },
                    onStateChange = { state ->
                        val previousState = _uiState.value.connection
                        _uiState.value = _uiState.value.copy(connection = state)
                        
                        // Just went from "not connected" to "connected" -> kick off a game.
                        if (state == ConnectionState.Connected && previousState != ConnectionState.Connected) {
                            startGame()
                            resetTimeout()
                        } else if (state is ConnectionState.Failed || state == ConnectionState.Disconnected) {
                            stopTimeout()
                        }
                    },
                )
            } catch (e: Exception) {
                Log.e("BiteBoundViewModel", "Connection failed", e)
                _uiState.value = _uiState.value.copy(
                    connection = ConnectionState.Failed(e.message ?: "Unknown error")
                )
            }
        }
    }

    private fun resetTimeout() {
        timeoutJob?.cancel()
        timeoutJob = viewModelScope.launch {
            delay(GameConfigConstants.TELEMETRY_TIMEOUT_MS)
            disconnect()
        }
    }

    private fun stopTimeout() {
        timeoutJob?.cancel()
        timeoutJob = null
    }

    fun disconnect() {
        stopTimeout()
        mqtt.disconnect()
        _uiState.value = _uiState.value.copy(
            connection = ConnectionState.Disconnected,
            telemetry = null,
            messageCount = 0,
        )
    }

    fun startGame(
        playerName: String = _uiState.value.credentials.playerName,
        gameId: Int = _uiState.value.credentials.gameId,
        cookiesCount: Int = _uiState.value.credentials.cookiesCount,
        wallThickness: Int = _uiState.value.credentials.wallThickness,
        imuSensitivity: Double = _uiState.value.credentials.imuSensitivity,
        restitution: Double = _uiState.value.credentials.restitution,
        emaAlpha: Double = _uiState.value.credentials.emaAlpha,
        deadzone: Double = _uiState.value.credentials.deadzone,
    ) {
        mqtt.publish(
            _uiState.value.credentials.commandTopic,
            GameCommand.start(
                playerName, gameId, cookiesCount, wallThickness,
                imuSensitivity, restitution, emaAlpha, deadzone
            ),
        )
    }

    fun stopGame() {
        val credentials = _uiState.value.credentials
        val telemetry = _uiState.value.telemetry
        val playerName = telemetry?.config?.playerName ?: credentials.playerName
        val gameId = telemetry?.config?.gameId ?: credentials.gameId
        val cookiesCount = telemetry?.config?.targetCookies ?: credentials.cookiesCount
        val wallThickness = telemetry?.config?.wallThicknessPx ?: credentials.wallThickness
        
        mqtt.publish(
            credentials.commandTopic,
            GameCommand.stop(playerName, gameId, cookiesCount, wallThickness),
        )
    }

    fun resumeGame(playerName: String) {
        val credentials = _uiState.value.credentials
        val telemetry = _uiState.value.telemetry
        val gameId = telemetry?.config?.gameId ?: 1
        val cookiesCount = telemetry?.config?.targetCookies ?: 10
        mqtt.publish(
            credentials.commandTopic,
            GameCommand.resume(playerName, gameId, cookiesCount),
        )
    }

    override fun onCleared() {
        mqtt.disconnect()
        super.onCleared()
    }
}
