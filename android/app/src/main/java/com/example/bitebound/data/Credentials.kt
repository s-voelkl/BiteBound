package com.example.bitebound.data

import android.content.Context

/**
 * MQTT connection settings. The broker host, port and topics are pre-filled
 * with the BiteBound project defaults (the same HiveMQ Cloud instance the
 * Node-RED dashboard uses) so the user normally only has to add a username and
 * password.
 */
data class Credentials(
    val host: String = DEFAULT_HOST,
    val port: Int = DEFAULT_PORT,
    val username: String = DEFAULT_USERNAME,
    val password: String = "",
    val telemetryTopic: String = DEFAULT_TELEMETRY_TOPIC,
    val commandTopic: String = DEFAULT_COMMAND_TOPIC,
) {
    val isComplete: Boolean
        get() = host.isNotBlank() && port in 1..65535 &&
            username.isNotBlank() && password.isNotBlank() &&
            telemetryTopic.isNotBlank() && commandTopic.isNotBlank()

    companion object {
        const val DEFAULT_HOST = "61775de8f674480cae63ae7690a8ffae.s1.eu.hivemq.cloud"
        const val DEFAULT_PORT = 8883
        const val DEFAULT_USERNAME = "BiteBound"
        const val DEFAULT_TELEMETRY_TOPIC = "mauc2026/group_03/game/telemetry"
        const val DEFAULT_COMMAND_TOPIC = "mauc2026/group_03/game/command"
    }
}

/**
 * Persists [Credentials] in SharedPreferences so they survive app restarts —
 * "add credentials once" is the whole point.
 */
class CredentialsStore(context: Context) {
    private val prefs = context.getSharedPreferences("bitebound_mqtt", Context.MODE_PRIVATE)

    fun load(): Credentials = Credentials(
        host = prefs.getString(KEY_HOST, Credentials.DEFAULT_HOST)!!,
        port = prefs.getInt(KEY_PORT, Credentials.DEFAULT_PORT),
        username = prefs.getString(KEY_USERNAME, Credentials.DEFAULT_USERNAME)!!,
        password = prefs.getString(KEY_PASSWORD, "")!!,
        telemetryTopic = prefs.getString(KEY_TELEMETRY, Credentials.DEFAULT_TELEMETRY_TOPIC)!!,
        commandTopic = prefs.getString(KEY_COMMAND, Credentials.DEFAULT_COMMAND_TOPIC)!!,
    )

    fun save(credentials: Credentials) {
        prefs.edit()
            .putString(KEY_HOST, credentials.host)
            .putInt(KEY_PORT, credentials.port)
            .putString(KEY_USERNAME, credentials.username)
            .putString(KEY_PASSWORD, credentials.password)
            .putString(KEY_TELEMETRY, credentials.telemetryTopic)
            .putString(KEY_COMMAND, credentials.commandTopic)
            .apply()
    }

    private companion object {
        const val KEY_HOST = "host"
        const val KEY_PORT = "port"
        const val KEY_USERNAME = "username"
        const val KEY_PASSWORD = "password"
        const val KEY_TELEMETRY = "telemetry_topic"
        const val KEY_COMMAND = "command_topic"
    }
}
