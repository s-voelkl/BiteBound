import json
import time
import random
import ssl
import paho.mqtt.client as mqtt

# Load configuration from config.json
with open("config.json", "r") as f:
    config = json.load(f)

broker = config.get("broker", "example.hivemq.cloud")
port = config.get("port", 8883)
username = config.get("username", "")
password = config.get("password", "")
topic = config.get("topic", "mauc2026/group_xx/game/telemetry")
client_id = config.get("client_id", "python_mock_sender")


# Callback for successful connection
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected successfully to MQTT Broker!")
    else:
        print(f"Failed to connect, return code {rc}")


# Initialize client
client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION1, client_id)  # type: ignore

# Set authentication and TLS -> Required for HiveMQ Cloud
if username and password:
    client.username_pw_set(username, password)

client.tls_set(tls_version=ssl.PROTOCOL_TLS_CLIENT)
client.on_connect = on_connect

# Establish connection
client.connect(broker, port, 60)
client.loop_start()

start_time = time.time()
cookies_collected = 0
cookies_target = 1000

try:
    print(f"Start publishing to topic '{topic}'...")
    while True:
        elapsed = time.time() - start_time

        # Simulate collecting cookies
        if (
            cookies_collected < cookies_target and random.random() < 0.025
        ):  # 2.5% chance to collect a cookie every 0.5s
            cookies_collected += 1

        # Generate dynamic data structure based on publish_example.json
        payload = {
            "device": {
                "client_id": "BiteBound_ESP32_G03",
                "hardware": "Waveshare ESP32-S3 1.69inch",
                "firmware_version": "1.0.0",
                "uptime_ms": int(elapsed * 1000),
                "wifi_ssid": "MyWiFiNetwork",
            },
            "config": {
                "game_id": 1,
                "player_name": "Player 1",
                "target_cookies": cookies_target,
                "screen_width": 240,
                "screen_height": 280,
                "wall_thickness_px": 6,
            },
            "state": {
                "status": "running"
                if cookies_collected < cookies_target
                else "finished",
                "cookies_collected": cookies_collected,
                "cookies_remaining": cookies_target - cookies_collected,
                "current_round": 1,
                "elapsed_time_sec": round(elapsed, 1),
            },
            "physics": {
                "ball_pos_x": round(random.uniform(10, 230), 2),
                "ball_pos_y": round(random.uniform(10, 270), 2),
                "velocity_x": round(random.uniform(-5.0, 5.0), 2),
                "velocity_y": round(random.uniform(-5.0, 5.0), 2),
                "acc_x": round(random.uniform(-1.0, 1.0), 2),
                "acc_y": round(random.uniform(-1.0, 1.0), 2),
                "collision_detected": random.choice([True, False, False, False]),
            },
            "sensors": {
                "accel_x": round(random.uniform(-1.0, 1.0), 2),
                "accel_y": round(random.uniform(-1.0, 1.0), 2),
                "accel_z": round(random.uniform(9.0, 10.0), 2),
                "gyro_x": round(random.uniform(-0.1, 0.1), 3),
                "gyro_y": round(random.uniform(-0.1, 0.1), 3),
                "gyro_z": round(random.uniform(-0.1, 0.1), 3),
            },
        }

        # Publish
        result = client.publish(topic, json.dumps(payload), qos=1, retain=False)
        status = result[0]
        if status == 0:
            print(
                f"Sent: {payload['physics']['ball_pos_x']}x, {payload['physics']['ball_pos_y']}y | Cookies: {cookies_collected}"
            )
        else:
            print(f"Error while sending ({status})")

        time.sleep(0.5)  # Sends an update every 0.5 seconds

except KeyboardInterrupt:
    print("\nStopping script...")
    client.loop_stop()
    client.disconnect()
