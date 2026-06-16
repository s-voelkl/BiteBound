package com.example.bitebound

import android.app.Application
import androidx.lifecycle.AndroidViewModel
import com.example.bitebound.data.Credentials
import com.example.bitebound.data.CredentialsStore
import com.example.bitebound.data.GameCommand
import com.example.bitebound.data.Telemetry
import com.example.bitebound.mqtt.ConnectionState
import com.example.bitebound.mqtt.MqttManager
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow

/** Everything the dashboard needs to render, in one immutable snapshot. */
data class UiState(
    val credentials: Credentials = Credentials(),
    val connection: ConnectionState = ConnectionState.Disconnected,
    val telemetry: Telemetry? = null,
    val messageCount: Int = 0,
)

class BiteBoundViewModel(app: Application) : AndroidViewModel(app) {

    private val store = CredentialsStore(app)
    private val mqtt = MqttManager()

    private val _uiState = MutableStateFlow(UiState(credentials = store.load()))
    val uiState: StateFlow<UiState> = _uiState.asStateFlow()

    /** Connect using [credentials], persisting them for next launch. */
    fun connect(credentials: Credentials) {
        store.save(credentials)
        _uiState.value = _uiState.value.copy(
            credentials = credentials,
            connection = ConnectionState.Connecting,
        )
        mqtt.connect(
            credentials = credentials,
            onMessage = { raw ->
                Telemetry.parse(raw)?.let { telemetry ->
                    _uiState.value = _uiState.value.copy(
                        telemetry = telemetry,
                        messageCount = _uiState.value.messageCount + 1,
                    )
                }
            },
            onStateChange = { state ->
                _uiState.value = _uiState.value.copy(connection = state)
            },
        )
    }

    fun disconnect() {
        mqtt.disconnect()
        _uiState.value = _uiState.value.copy(
            connection = ConnectionState.Disconnected,
            telemetry = null,
            messageCount = 0,
        )
    }

    fun startGame(playerName: String, cookiesCount: Int) {
        val credentials = _uiState.value.credentials
        val gameId = _uiState.value.telemetry?.config?.gameId ?: 1
        mqtt.publish(
            credentials.commandTopic,
            GameCommand.start(playerName, gameId, cookiesCount),
        )
    }

    fun stopGame(playerName: String) {
        val credentials = _uiState.value.credentials
        val telemetry = _uiState.value.telemetry
        val gameId = telemetry?.config?.gameId ?: 1
        val cookiesCount = telemetry?.config?.targetCookies ?: 10
        mqtt.publish(
            credentials.commandTopic,
            GameCommand.stop(playerName, gameId, cookiesCount),
        )
    }

    override fun onCleared() {
        mqtt.disconnect()
        super.onCleared()
    }
}
