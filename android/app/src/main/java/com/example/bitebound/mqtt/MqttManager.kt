package com.example.bitebound.mqtt

import com.example.bitebound.data.Credentials
import com.hivemq.client.mqtt.MqttClient
import com.hivemq.client.mqtt.datatypes.MqttQos
import com.hivemq.client.mqtt.mqtt5.Mqtt5AsyncClient
import java.nio.charset.StandardCharsets
import java.util.UUID

/** High-level outcome of an MQTT connection attempt, surfaced to the UI. */
sealed interface ConnectionState {
    data object Disconnected : ConnectionState
    data object Connecting : ConnectionState
    data object Connected : ConnectionState
    data class Failed(val reason: String) : ConnectionState
}

/**
 * Thin wrapper around the HiveMQ MQTT 5 client. Handles a TLS + username/password
 * connection to HiveMQ Cloud, subscribes to the telemetry topic and publishes
 * game commands. All client callbacks arrive on Netty threads, so the manager
 * only forwards raw values to the supplied listeners — state fan-out happens in
 * the ViewModel.
 */
class MqttManager {

    private var client: Mqtt5AsyncClient? = null

    /**
     * Connects to the broker described by [credentials] and subscribes to the
     * telemetry topic.
     *
     * @param onMessage  invoked with each raw telemetry payload (off the main thread)
     * @param onStateChange invoked as the connection progresses
     */
    fun connect(
        credentials: Credentials,
        onMessage: (String) -> Unit,
        onStateChange: (ConnectionState) -> Unit,
    ) {
        disconnect()
        onStateChange(ConnectionState.Connecting)

        val mqttClient = MqttClient.builder()
            .useMqttVersion5()
            .identifier("BiteBound-Android-" + UUID.randomUUID())
            .serverHost(credentials.host)
            .serverPort(credentials.port)
            .sslWithDefaultConfig() // HiveMQ Cloud is TLS-only on 8883
            .automaticReconnectWithDefaultConfig()
            .addDisconnectedListener { ctx ->
                // Only surface a hard failure when we are not auto-reconnecting.
                if (ctx.reconnector.attempts == 0) {
                    onStateChange(
                        ConnectionState.Failed(
                            ctx.cause.message ?: "Connection lost",
                        ),
                    )
                }
            }
            .buildAsync()
        client = mqttClient

        mqttClient.connectWith()
            .simpleAuth()
            .username(credentials.username)
            .password(credentials.password.toByteArray(StandardCharsets.UTF_8))
            .applySimpleAuth()
            .cleanStart(true)
            .keepAlive(30)
            .send()
            .whenComplete { _, throwable ->
                if (throwable != null) {
                    onStateChange(
                        ConnectionState.Failed(
                            throwable.message ?: "Could not connect",
                        ),
                    )
                    return@whenComplete
                }
                subscribe(mqttClient, credentials.telemetryTopic, onMessage, onStateChange)
            }
    }

    private fun subscribe(
        mqttClient: Mqtt5AsyncClient,
        topic: String,
        onMessage: (String) -> Unit,
        onStateChange: (ConnectionState) -> Unit,
    ) {
        mqttClient.subscribeWith()
            .topicFilter(topic)
            .qos(MqttQos.AT_LEAST_ONCE)
            .callback { publish ->
                onMessage(String(publish.payloadAsBytes, StandardCharsets.UTF_8))
            }
            .send()
            .whenComplete { _, throwable ->
                onStateChange(
                    if (throwable != null) {
                        ConnectionState.Failed(
                            throwable.message ?: "Subscription failed",
                        )
                    } else {
                        ConnectionState.Connected
                    },
                )
            }
    }

    /** Publishes a game command payload to [topic]. No-op when not connected. */
    fun publish(topic: String, payload: String) {
        client?.publishWith()
            ?.topic(topic)
            ?.qos(MqttQos.AT_LEAST_ONCE)
            ?.retain(false)
            ?.payload(payload.toByteArray(StandardCharsets.UTF_8))
            ?.send()
    }

    fun disconnect() {
        client?.let { c ->
            runCatching { c.disconnect() }
        }
        client = null
    }
}
