#include "src/network/time/TimeManager.h"
#include "src/network/wifi/WifiManager.h"
#include "src/network/mqtt/MqttManager.h"
#include "src/sensors/SensorManager.h"
#include "src/physics/PhysicsEngine.h"
#include "src/physics/BorderCollider.h"
#include "src/game/CookieField.h"
#include "src/game/RectCookieSpawner.h"
#include "config.h"
#include <ArduinoJson.h>
#include <AUnit.h>

#ifndef RUN_TESTS
#define RUN_TESTS 0
#endif

// ---- Spielobjekte (single-threaded, keine Tasks/Queues/Semaphore) ----
PhysicsEngine     engine;
BorderCollider    borders(display_width, display_height);
RectCookieSpawner cookieSpawner(display_width, display_height, 3.0f);
CookieField       cookies;
PhysicsBody       ball;
bool     gameRunning      = false;
uint32_t prevMs           = 0;
uint32_t lastTelemetryMs  = 0;

// Dummy-Sensor: ersetzt den QMI8658. Liefert eine langsam rotierende Neigung,
// damit der Ball ohne echten IMU über das Feld kreist (z. B. auf dem C3-Zero).
SensorData readDummySensor() {
  SensorData d;
  const float t = millis() / 1000.0f;
  d.accelerometerX = 0.6f * cosf(t);   // simulierte Neigung [g]
  d.accelerometerY = 0.6f * sinf(t);
  d.accelerometerZ = 9.81f;
  d.gyroscopeX = 0.0f; d.gyroscopeY = 0.0f; d.gyroscopeZ = 0.0f;
  d.button = false;
  return d;
}

void startGame(uint16_t targetCookies) {
  PhysicsParams p;
  p.sensitivity = 200.0f;                 // g -> px/s^2 (auf echter HW feinjustieren)
  engine.setParams(p);
  engine.reset();
  ball.x = display_width  * 0.5f;
  ball.y = display_height * 0.5f;
  ball.vx = ball.vy = 0.0f;
  cookies.start(3, targetCookies, cookieSpawner, ball);  // 3 sichtbar, Ziel = count
  gameRunning = true;
  prevMs = millis();
  Serial.println("Game started.");
}

String buildTelemetry(const SensorData &s, Vec2 accel, bool collided) {
  StaticJsonDocument<768> doc;
  JsonObject st = doc.createNestedObject("state");
  st["status"]            = gameRunning ? "running" : "finished";
  st["cookies_collected"] = cookies.collected();
  st["cookies_remaining"] = cookies.remaining();
  JsonObject ph = doc.createNestedObject("physics");
  ph["ball_pos_x"] = ball.x;  ph["ball_pos_y"] = ball.y;
  ph["velocity_x"] = ball.vx; ph["velocity_y"] = ball.vy;
  ph["acc_x"] = accel.x;      ph["acc_y"] = accel.y;
  ph["collision_detected"] = collided;
  JsonObject se = doc.createNestedObject("sensors");
  se["accel_x"] = s.accelerometerX; se["accel_y"] = s.accelerometerY; se["accel_z"] = s.accelerometerZ;
  String out; serializeJson(doc, out); return out;
}

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 BiteBound starting...");
#if RUN_TESTS
  delay(3000);
  Serial.println("Starting AUnit tests...");
  aunit::TestRunner::exclude("*");
  aunit::TestRunner::include("physics_*");
  aunit::TestRunner::include("cookie_*");
  aunit::TestRunner::setTimeout(0);
#else
  wifiManager.connect();              // 1. WLAN
  timeManager.sync();                 // 2. Zeit (für TLS + Timestamps)
  sensorManager.begin();             // 3. Sensoren init
  mqttManager.begin();               // 4. MQTT-Client init
  randomSeed(micros());              // 5. Zufall für Keks-Positionen
  startGame(default_cookies_count);  // 6. Spiel starten (hier hart codiert)
#endif
}

void loop() {
#if RUN_TESTS
  aunit::TestRunner::run();
#else
  // 1) MQTT am Leben halten (und ggf. Befehle empfangen)
  mqttManager.connect();
  mqttManager.loop();

  // 2) Zeitschritt bestimmen
  const uint32_t now = millis();
  float dt = (now - prevMs) / 1000.0f;
  prevMs = now;

  SensorData s = readDummySensor();   // 3) Sensor lesen (Dummy)
  Vec2 accel(0.0f, 0.0f);
  bool collided = false;

  if (gameRunning && dt > 0.0f) {
    accel    = engine.inputAccel(s.accelerometerX, s.accelerometerY); // 4) -> Beschleunigung
    collided = engine.step(ball, accel, dt, borders);                 // 5) Physik + Rand-Abprall
    cookies.checkPickup(ball);                                        // 6) Kekse: Score + Respawn
    if (cookies.finished()) {                                         // 7) Siegbedingung
      gameRunning = false;
      Serial.println("Game finished!");
    }
    // 8) TODO: hier auf das Display rendern (GFX)
  }

  // 9) Telemetrie ~2 Hz senden
  if (now - lastTelemetryMs >= 500) {
    lastTelemetryMs = now;
    mqttManager.publish(mqtt_telemetry_topic, buildTelemetry(s, accel, collided), mqtt_retain);
  }

  delay(20);   // ~50 Hz Takt
#endif
}