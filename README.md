# BiteBound

## Before making this repo public

Before making this repository public, the following steps have to be done:

- Delete examples folder, as we dont have the rights to use or publish this information.
- Restructure folders and files: Make ``src`` with each Subproject and ``docs`` with extensive documentation and tex-files.
- Delete prompts folder

## GitHub Repository

The GitHub repository for this project is located at: [https://github.com/s-voelkl/BiteBound](https://github.com/s-voelkl/BiteBound).

## License

MIT License

## Usage

See [Usage Guide](USAGE.md) for detailed instructions on how to set up and run the project.

## Physik Simulation

2D Euler-Integration.
Fine-tuning nötig.
Bibliothek für ESP32 vorhanden?
Rauschen vermindern: Thresholds einbauen für Aktivierung (Deadzone)
Tiefpassfilter (SMA / EMA): Neue Werte basieren ein wenig auf die alten Werte --> Glättung
Kollisionsverhalten: Konstante: v[neu] = -v[alt] * e. e = 0.5 als Dämpfung.

### Labyrinth-Generierung

Randomized Depth-First Search als kontrollierter Backtracking-Algorithmus (DFS).
Zellen mit Wänden dazwischen.

### Ablauf

1. Sensordaten lesen (Read) --> Tiefpassfilter, Deadzone
2. Physik aktualisieren --> Simulation 1 Schritt laufen lassen. Beinhaltet Kollisionsverhalten.
3. Rendern --> GFX-Bibliothek ansteuern.
4. [alle 0.5s] MQTT senden (async!)

### ESP32-S3 Dual Core Prozessor: 2 Threads möglich

- Core 1: Game Core mit 50Hz: Sensorwerte, Physik, Display
- Core 0: Prädestiniert für MQTT mit 2Hz. --> kein Delay

## MQTT

MQTT möglichst viel von Sensiq Copy-Pasten.
Frequenz 50Hz. millis() Funktion für einfachere Ansteuerung.
Mutex: SemaphoreHandle_t (Mutex) für globale Variablen (Kugelposition, ...)
Queue: für Game Command von Dashboard für "START, STOP, ..."

### MQTT Topics

- Base Topic: mauc2026/group_03/
- .../game/command: Spiel auswählen, starten, Parameter (Labyrinth Wandstärke, Anzahl Kekse, Spielername) an ESP32
  Bsp: {"game_id": 1, "status": "RUNNING", "player_name": "Alex", "score": 4, "total_cookies": 10, "round": 1}
- .../game/telemetry: Spielstandsanzeige (Bewegungssensorwerte, Physiksimulationswerte, Zeit, Spielstand (Punkte, Runden), etc.) an Dashboard
  Bsp: {"ball_x": 112, "ball_y": 145, "tilt_x": 0.15, "tilt_y": -0.34}
- ...test: Für Testzwecke und Debugging, z.B. bei Connection.

Im Allgemeinen mit QoS 1 und Retain Flag auf False senden.

## Folder Structure

Gruppe_03/
├── Gruppe_03.pdf             # Main documentation (compiled from LaTeX)
├── android/                  # Android Studio project root
│   ├── app/                  # Application source files (Kotlin/Java)
│   │   └── src/              # App logic and UI layouts
│   └── build.gradle          # Exclude the 'app/build' folder on submission!
├── esp32/                    # Arduino project folder
│   ├── esp32.ino             # Main sketch file
│   ├── game_physics.cpp      # Physics engine implementation
│   ├── game_physics.h
│   ├── maze_generator.cpp    # DFS Maze generation algorithm
│   ├── maze_generator.h
│   ├── ........
│   └── wifi_mqtt_secrets.h   # Network credentials (git-ignored during dev)
├── nodered/                  # Node-RED dashboard folder
│   └── flow.json             # Exported Node-RED flow
├── prompts/                  # LLM chat transcripts & history logs
│   └── chat_history.md
├── screenshots/              # Game UI, Dashboard layouts, and flow screenshots
└── tex/                      # LaTeX source tracking
    ├── projektarbeit.tex     # Main LaTeX document template
    └── quellen.bib           # Zotero / JabRef bibliography

## Projectmanagement

Spiel 1 & 2:
Benutzen beide selbe Grundlage (Sensorwerte, MQTT, WiFi, Cookies, Display)

### Erweiterungen

Spiel 1:

- Ghost Enemy (A-Star Algorithm)

Spiel 2:

- Hindernisse, Löcher, ...

Beide Spiele:

- Vibrationsbuzzer
- Sound spielen (tricky!!!)

### Aufgaben

#### Hardware

- ESP32 Setup
- MQTT + WiFi Verbindung, JSON-Parsing (In + Out)
- Sensorwerte lesen, glätten, usw.
- Physiksimulation (Geschwindigkeit, Beschleunigung, Kollision, Cookie-Kollision, ...)
- Maze Generation
- Cookies generieren
- Displaylogik mit GFX-Bibliothek
- Spiel 1: Labyrinth
- Spiel 2: Kugel auf Bildschirm
- Threads aufsetzen, Safety testen
- AUnit Unit Tests

#### Nodered

- MQTT Signale lesen
- Dashboard für Anzeige von Punkten, Name, Runden, Zeit, Physikdaten, Sensorwerte, ...
- Eingabemaske für Spielername, Keksanzahl, Wandstärke, Buttons für Start/Stop

#### Android [Ausweichoption]: MQTT Dash

- Verbindung aufsetzen
- Subscribe: Daten erhalten und darstellen
- Publish: Game starten

#### Android [Primäroption]: Android-App in Kotlin

- Repo aufsetzen
- Credentials file
- HiveMQ-Bibliothek für MQTT benutzen
- Subscribe: Live-Anzeige der Daten
- Publish: Senden von Befehlen (+Eingabemaske für Inputs)

#### TechRep

### TechRep Specs

- 10 Seiten, 3-4 Screenshots
- Node-Red-Workflow (Screenshot)
- ESP32-Code in Anhang

Struktur:

- Introduction:
  - Intro
  - Mission Statement
  - Motivation
  - Document Structure
- Related Work:
  - tbd
- Projektmanagement:
  - SMART
  - User Stories (Anforderungen)
  - MVP
  - Erweiterungen
  - Aufgabenverteilung (Guidelines/Standards, PRs, Code Coverage, Git, ...)
 -Technical Concept:
  - Grobarchitektur (+Bild)
  - Komponenten (Hardware, Nodered, Android)
- Hardware:
  - Komponenten (ESP32) im Detail
  - Sensorerfassung
  - Physiksimulation (+Formeln)
  - Display (+Screenshots)
  - MQTT Topics + Payloads (+listing JSON)
- Nodered:
  - Workflow (+Screenshot)
  - Dashboard + Userinput (+Screenshot)
- Android App:
  - Softwarekomponenten, Klassen, Views
  - Dashboard + UserInput (+Screenshot)
- Evaluation:
  - Probleme + Lösungen
  - Code Coverage
  - (Cost Estimation)
- Summary:
  - Summary + Future Work
- Anhang:
  - Prompts
  - ESP32 Code

### Aufgabenverteilung

ESP:
Simon: Setup MQTT + WiFi + AUnit (von Sensiq), Maze-Generierung
Schieder: Repo-Setup, Physiksimulation, Cookies, Sensorwerte

NodeRed:
Simon: Setup

Android:
Schieder: Setup

### Guidelines

- Test Coverage: ESP32 ohne Display 80%, Android 40% max., NodeRed nichts.
- main Branch Protection, mit PRs

## Documentation

### ESP32-S3-Touch-LCD-1.69 Features

See: <https://docs.waveshare.com/ESP32-S3-Touch-LCD-1.69>

- ESP32-S3R8 Wi-Fi and Bluetooth SoC, 240 MHz, 8MB stacked PSRAM
- W25Q128JVSIQ 16MB NOR Flash
- PCF85063 RTC clock chip
- QMI8658 6-axis IMU (3-axis gyroscope + 3-axis accelerometer)
- ETA6098 high-efficiency Li-battery charging chip
- MX1.25 Li-battery connector MX1.25 2P connector for 3.7V Li-battery, supports charging/discharging
- Type-C port ESP32-S3 USB, for program uploading and log printing
- Buzzer audible peripheral
- RTC battery connector for rechargeable RTC battery, supports charging/discharging
- Onboard chip antenna supports 2.4 GHz Wi-Fi (802.11 b/g/n) and Bluetooth® 5 (LE)
- Power function button supports power-on detection, single-click, double-click, multi-click and long-press
- BOOT button
- RST reset button

Top side: Where the USB-C port is left-sided and the top display frame is smaller.

Acceleration measurements:

- X: Front/Back movement. Positive when moving forward. Default of 0.0
- Y: Left/Right movement. Positive when moving right. Default of 0.0
- Z: Up/Down movement. Down = more negative. Default of -1G.

Gyroscope measurements

- X: Left/Right rotation --> Roll
- Y: Forward/Backward rotation --> Pitch
- Z: Clockwise/Counterclockwise rotation --> Yaw

### Sensor Handling

Implemented in `esp32/src/sensors/SensorManager.h` and `esp32/src/sensors/SensorManager.cpp`.

- Sources: QMI8658 IMU (I2C), battery ADC, power button GPIO
- `SensorData` fields: `timestamp`, `accelerometerX/Y/Z`, `gyroscopeX/Y/Z`, `batteryVoltage`, `button`
- No touch fields in the current implementation
- `begin()` is safe to call once; `read()` keeps last IMU values if no fresh IMU data is available
- `readMock()` returns bounded test values (battery within configured mock min/max)

`esp32/src/sensors/TestSensorManager.cpp` covers init state, valid `read()` output, and `readMock()` range checks.

#### Basic Usage

```cpp
#include "src/sensors/SensorManager.h"

sensorManager.begin();

if (sensorManager.isInitialized()) {
    SensorData sensorData = sensorManager.read();
    // publish or process sensorData
}

// Optional test/mock path without relying on hardware values
// SensorData sensorData = sensorManager.readMock();
```

### Telemetry JSON Builder

The JSON builder compiles comprehensive telemetry data from sensors, game state, physics simulation, and device information into a structured JSON payload suitable for MQTT transmission.

Implemented in `esp32/src/network/json-builder/JsonBuilder.h` and `esp32/src/network/json-builder/JsonBuilder.cpp`.

- **Input**: `TelemetryData` struct containing device info, game config, game state, physics state, and sensor readings
- **Output**: Formatted JSON string ready for MQTT publication
- **Structure**: Hierarchical JSON with categories: `device`, `config`, `state`, `physics`, and `sensors`

#### JSON Payload Structure

```json
{
  "device": {
    "client_id": "BiteBound-ESP32-S3-001",
    "hardware": "Waveshare ESP32-S3 1.69inch",
    "firmware_version": "1.0.0",
    "uptime_ms": 745200,
    "wifi_ssid": "MyWiFiNetwork"
  },
  "config": {
    "game_id": 1,
    "player_name": "Player 1",
    "target_cookies": 15,
    "screen_width": 240,
    "screen_height": 280,
    "wall_thickness_px": 6 // min: 6, max: 40 --> ball size 1/2
  },
  "state": {
    "status": "running", // idle, running, completed
    "cookies_collected": 4,
    "cookies_remaining": 11,
    "current_round": 2, // game init: 1. game change: reset to 1. 
    "elapsed_time_sec": 42.8
  },
  "physics": {
    "ball_pos_x": 112.45,
    "ball_pos_y": 145.2,
    "velocity_x": 1.85,
    "velocity_y": -0.92,
    "acc_x": 0.15,
    "acc_y": -0.34,
  },
  "sensors": {
    "accel_x": 0.12,
    "accel_y": -0.08,
    "accel_z": 9.81,
    "gyro_x": 0.02,
    "gyro_y": -0.01,
    "gyro_z": 0.005,
    "battery_voltage": 4.2,
    "button": false
  }
}
```

#### Basic Usage

```cpp
#include "src/network/json-builder/JsonBuilder.h"
#include "src/sensors/SensorManager.h"

// Read sensor data
SensorData sensorData = sensorManager.read();

// Populate telemetry data struct
TelemetryData telemetry;
telemetry.client_id = device_id;
telemetry.hardware = "Waveshare ESP32-S3 1.69inch";
// ...

// Build and publish JSON
String payload = buildTelemetryJson(telemetry);
mqttManager.publish(mqtt_telemetry_topic, payload.c_str(), mqtt_retain);
```

### MQTT Communication

#### Topics

#### Communication Logic

On game start or change: game round resets to 1.
