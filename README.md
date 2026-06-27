# BiteBound

## Before making this repo public

Before making this repository public, the following steps have to be done:

- Delete examples folder, as we dont have the rights to use or publish this information.
- Restructure folders and files: Make ``src`` with each Subproject and ``docs`` with extensive documentation and tex-files.
- Delete prompts folder

## GitHub Repository

The GitHub repository for this project is located at: [https://github.com/s-voelkl/BiteBound](https://github.com/s-voelkl/BiteBound).

## License

The MIT License (MIT) applies to this project. See the [LICENSE](LICENSE) file for details.

## Usage

See [Usage Guide](USAGE.md) for detailed instructions on how to set up and run the project.

## Architecture Overview

The ESP32 firmware is split into self-contained, unit-tested modules under `esp32/src/`, orchestrated from the main sketch `esp32/esp32.ino`. All tunable values and hardware pins are centralized in `esp32/config.h`.

| Module | Path | Responsibility |
| --- | --- | --- |
| Sensors | `src/sensors/` | Reads and smooths the IMU, battery and button (`SensorManager`, `SensorData`). |
| Physics | `src/physics/` | Game-agnostic 2D simulation, integration and collision (`PhysicsEngine`, `PhysicsBody`, `ICollider`, `BorderCollider`, `Vec2`). |
| Game | `src/game/` | Shared cookie/score logic (`Cookie`, `CookieField`, `ICookieSpawner`, `RectCookieSpawner`, `MazeCookieSpawner`, `GameState`). |
| Maze | `src/maze/` | Procedural DFS maze generation (`MazeManager`). |
| Graphics | `src/graphics/` | Optimized 50Hz rendering on the ST7789 display (`GraphicsManager`). |
| Network | `src/network/` | WiFi, NTP time, secure MQTT and telemetry JSON (`WifiManager`, `TimeManager`, `MqttManager`, `JsonBuilder`). |

See the component diagram in [diagrams/src/architecture.puml](diagrams/src/architecture.puml).

### Concurrency Model (ESP32-S3 Dual Core)

The ESP32-S3 is a dual-core SoC, which lets the latency-sensitive game loop run independently from the network stack:

- **Core 1 — Game Core (~50Hz / 20ms budget):** sensor read, physics step, collision, cookie pickup,  game state and display rendering.
- **Core 0 — Network Core (~2Hz):** MQTT publish/receive and keep-alive, so blocking network I/O never stalls the game loop.

Shared state (ball position, game state, runtime config received over MQTT) is protected with a FreeRTOS mutex (`SemaphoreHandle_t`). Incoming game commands from the dashboard (START / STOP / parameter changes) are passed to the game core via a queue.

### Game Loop

1. **Read sensors** — IMU tilt is low-pass filtered (EMA) and a deadzone is applied.
2. **Step physics** — one `PhysicsEngine::step()` advances the ball, clamps speed, sub-steps to avoid tunneling and resolves collisions against the active `ICollider`.
3. **Update game** — `CookieField::checkPickup()` scores and respawns cookies; `GameState` is updated.
4. **Render** — `GraphicsManager::update()` performs a partial (dirty-rect) or full redraw.
5. **Publish telemetry** — every ~0.5s the current snapshot is serialized via `buildTelemetryJson()` and published over MQTT (asynchronously on Core 0).

> Note: MQTT messages are sent with **QoS 1** and **retain = false** by default. Topic details and the full telemetry payload are documented in the [MQTT Communication](#mqtt-communication) and [Telemetry JSON Builder](#telemetry-json-builder) sections.

## Folder Structure

```text
BiteBound/
├── README.md                 # This documentation
├── USAGE.md                  # Setup & run instructions
├── android/                  # Android Studio project (Kotlin app)
├── diagrams/                 # PlantUML architecture diagrams
│   └── src/architecture.puml
├── esp32/                    # Arduino firmware
│   ├── esp32.ino             # Main sketch (setup/loop, Core 1 orchestration)
│   ├── config.h              # Central config: network, game, display, physics, pins
│   ├── wifi_mqtt_secrets.h   # Network/broker credentials (git-ignored during dev)
│   └── src/
│       ├── sensors/          # SensorManager, SensorData
│       ├── physics/          # PhysicsEngine, PhysicsBody, ICollider, BorderCollider, Vec2
│       ├── game/             # Cookie, CookieField, ICookieSpawner, RectCookieSpawner, MazeCookieSpawner, GameState
│       ├── maze/             # MazeManager (procedural DFS)
│       ├── graphics/         # GraphicsManager (ST7789 rendering)
│       └── network/          # wifi-connection/, time/, mqtt/, json-builder/
├── nodered/                  # Node-RED dashboard flow
├── prompts/                  # LLM chat transcripts (removed before publishing)
├── screenshots/              # UI / dashboard / flow screenshots
└── tex/                      # LaTeX report sources
```

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

<!-- update? -->

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

#### Android: Android-App in Kotlin

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

### Configuration (config.h)

All tunable parameters and hardware constants live in `esp32/config.h`, so behavior can be adjusted in one place without touching the module code. The values are grouped into the following categories:

- **Network & device identity** — device ID, hardware/firmware strings.
- **MQTT** — port, keep-alive, topics (command / telemetry / test), QoS and retain defaults.
- **Game** — default game duration, target cookie count, max visible/rendered cookies.
- **Display & colors** — screen dimensions, HUD header height, wall thickness and the cookie-themed RGB565 color palette.
- **Physics tuning** — IMU sensitivity, bounce restitution, EMA alpha, deadzone, max speed, linear damping, contact iterations.
- **Time / NTP** — NTP servers, GMT/DST offsets, epoch validity threshold.
- **Hardware pins** — I2C (IMU/touch), buttons, and the SPI LCD pins.
- **Analog & battery** — ADC reference/resolution, voltage divider and mock battery range.

Many physics values (e.g. sensitivity, restitution) are mirrored in `PhysicsParams` and can be adjusted at runtime via MQTT.

### Secrets (wifi_mqtt_secrets.h)

The `esp32/wifi_mqtt_secrets.h` file is git-ignored and contains the following sensitive values:

- WiFi SSID and password
- HiveMQ Cloud broker hostname, port, username and password
- HiveMQ Cloud CA certificate (PEM format)

A template file `esp32/wifi_mqtt_secrets.h.template` is provided to show the expected structure.

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

### Network Managers (WiFi, Time, MQTT)

Network connectivity is split into three single-responsibility managers under `esp32/src/network/`, each configured from `config.h` / `wifi_mqtt_secrets.h` and exposed as a shared global instance.

#### WiFi (`wifi-connection/WifiManager`)

Wraps the WiFi driver and manages the connection lifecycle.

- `connect()` — blocks until associated with the configured access point.
- `isConnected()` — current link state.
- `localIP()` — assigned IP address as a string.
- `getSSID()` — SSID of the connected network (published in telemetry).

#### Time (`time/TimeManager`)

Synchronizes the system clock via NTP. An accurate clock is required for ISO 8601 timestamps and to validate the broker's TLS certificate.

- `sync()` — performs NTP sync and blocks until a plausible time is set.
- `now()` — current Unix timestamp.
- `isSynchronized()` — whether the clock passed the configured epoch threshold.

#### MQTT (`mqtt/MqttManager`)

Manages the secure (TLS) MQTT connection to the HiveMQ Cloud broker via `PubSubClient`.

- `begin()` — installs the CA certificate, sets broker/keep-alive/callback/buffer (call after time sync).
- `connect()` — establishes (or re-establishes) the connection; verifies via the test topic.
- `loop()` — services incoming traffic and keep-alive (call every iteration).
- `publish(topic, payload, retain)` / `subscribe(topic, qos)` — send/receive payloads.
- `isConnected()`, `state()`, `disconnect()` — connection introspection and teardown.

Recommended defaults: **QoS 1** and **retain = false** (see `config.h`).

### Telemetry JSON Builder

The JSON builder compiles comprehensive telemetry data from sensors, game state, physics simulation, and device information into a structured JSON payload suitable for MQTT transmission.

Implemented in `esp32/src/network/json-builder/JsonBuilder.h` and `esp32/src/network/json-builder/JsonBuilder.cpp`.

- **Input**: `TelemetryData` struct containing device info, game config, game state, physics state, and sensor readings
- **Output**: Formatted JSON string ready for MQTT publication
- **Structure**: Hierarchical JSON with categories: `device`, `config`, `state`, `physics`, and `sensors`

#### JSON Telemetry Payload

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

##### 1. Command Topic

``mauc2026/group_03/game/command``: Android / NodeRED --> ESP32. Commands to start/stop the game, set parameters, and control game state.

##### 2. Telemetry Topic

``mauc2026/group_03/game/telemetry``: ESP32 --> Android / NodeRED. Publishes telemetry data including sensor readings, game state, and physics simulation results.

#### Communication Logic

On game start or change: game round resets to 1.

### Display Manager

SPI interface LCD display pins: see [library example](https://github.com/waveshareteam/ESP32-S3-Touch-LCD-1.69/blob/main/examples/Arduino/libraries/Mylibrary/pin_config.h)

#### Display Subsystem & Graphics Manager

The rendering engine for the 240x280 ST7789 display is managed by the `GraphicsManager` class. It is designed to sustain a stable **50Hz game loop (20ms time budget)** on Core 1 while preventing display tearing, flicker, or performance bottlenecks.

#### Architectural Overview

Updating a $240 \times 280$ display in 16-bit color (RGB565) requires pushing **131.25 KB** of data per frame. Over a standard, stable 27 MHz SPI bus, a full frame transmission takes **~40ms**, which physically violates the 20ms constraint of a 50Hz game loop.

To circumvent this hardware bottleneck, the `GraphicsManager` implements a hybrid rendering architecture:

1. **Full Redraw Mode (Round Start / State Transition):**
   When a new round begins or the game state changes, a full redraw is triggered. The manager clears the screen, draws the HUD, and renders the static maze. To optimize SPI bandwidth, the maze generator renders horizontal spans of identical pixel states using an RLE (Run-Length Encoding) algorithm rather than individual pixels.
2. **Partial Update Mode (50Hz Active Ticks):**
   During active gameplay, the screen is *never* fully re-cleared. Instead, the manager utilizes a **"Dirty Rectangles"** approach:
   - It calculates the bounding box of the sphere's *previous* position and overwrites only that region with the underlying static maze background.
   - It checks for and restores any active cookies that were overlapped by the sphere's previous position to prevent graphical clipping.
   - It handles cookie collection/respawns by locally drawing or erasing only those tiny $16 \times 16$ pixel regions.
   - It draws the sphere at its *new* position.

This reduces the active frame data payload from **131.25 KB to ~1 KB**, dropping display transmission overhead from **~40ms to under 1ms**.

1. **HUD Smart Caching:**
   Drawing text characters is computationally expensive. The manager checks if volatile variables (such as score or status) have updated, or if a full second has elapsed, before performing HUD redraws.

---

#### Basic Usage

##### 1. Initialization

Include the header and instantiate the class. The board dimensions are automatically retrieved from `config.h`.

```cpp
#include "src/graphics/GraphicsManager.h"

// Instantiate the display bus and driver as per the hardware setup
Arduino_DataBus *bus = new Arduino_ESP32SPI(pin_lcd_dc, pin_lcd_cs, pin_lcd_sck, pin_lcd_mosi);
Arduino_GFX *gfx = new Arduino_ST7789(bus, pin_lcd_rst /* RST */,0 /* rotation */, 
  true /* IPS */, display_width, display_height, 0, 20, 0, 0);

// Instantiate the Graphics Manager
GraphicsManager graphicsManager(display_width, display_height);
```

##### 2. Setup Hook

Attach your physical display instance during the system startup routine:

```cpp
void setup() {
    if (!gfx->begin()) {
      Serial.println("Failed to initialize GFX display!");
    }
    graphicsManager.begin(gfx);
    
    // Additional game setup...
}
```

##### 3. Loading Screen

A simple loading screen can be displayed while the game initializes:

```cpp
graphicsManager.drawLoadingScreen("Loading Game...");
```

##### 4. Execution

Invoke the `update()` method on every 20ms tick. The manager automatically determines whether a full redraw or a high-performance partial redraw is necessary based on state changes.

```cpp
  // 1. Update sensors, physics, collisions, game state, etc.
  // 2. Request screen update
  graphicsManager.update(
      mazeGrid,           // uint8_t array representing the wall/empty play grid
      play_width,         // Width of the play area
      play_height,        // Height of the play area
      simulationBall,     // PhysicsBody struct of the ball
      simulationCookies,  // Array of Cookie structs
      cookie_count,       // Total size of cookie array
      telemetryState      // GameState tracking meta values (round, time, score)
  );
```

### Maze Generation (Procedural DFS)

A procedural 2D maze generator has been integrated to build the game-board using an iterative, randomized Depth-First Search (DFS) algorithm (see AI prompt in [prompts/prompts.md](prompts/prompts.md)).

#### Implementation Summary

The maze generation is handled by the `MazeManager` class. Rather than using recursive calls — which risk exceeding the stack limits of an ESP32 task — the generation employs an iterative stack-based approach using standard library containers (`std::stack` and a flat `std::vector<bool>` for visited state tracking).

The algorithm treats the board as a grid of blocks with a uniform dimension defined by the wall thickness. It identifies reachable pathway nodes (cells) at odd block coordinates, procedurally carving paths (value `0`) through walls of Type 2 (value `2`) until all cells have been visited.

#### Technical Details

##### Grid Mapping & Sizing

- **Block Resolution:** The physical screen is divided into a grid of blocks, where each block is $W \times W$ pixels (with $W$ being the `wall_thickness_px`).
- **Symmetry Constraints:** For a maze to have uniform enclosing borders, the grid dimensions must be odd. If the calculated columns or rows of the block grid are even, the class dynamically reduces the active maze grid bounds by 1 (`mazeCols = gridCols - 1`).
- **Symmetric Centering:** Unused pixel remainders (due to uneven screen divisions or odd-dimension adjustments) are calculated and split evenly. This creates horizontal (`offsetX`) and vertical (`offsetY`) offsets to align the active maze perfectly within the display borders, distributing background padding symmetrically on opposite edges.

##### Coordinate Math

- **Cells (Path Nodes):** Located at odd block coordinates:
  $$gx = 2 \cdot cx + 1, \quad gy = 2 \cdot cy + 1$$

- **Intermediary Walls:** When moving from cell $(cx, cy)$ to an adjacent cell $(nx, ny)$, the intervening wall block is calculated and carved at:
  $$gx_{wall} = cx + nx + 1, \quad gy_{wall} = cy + ny + 1$$

##### Output Pixel Representations

The output buffer is populated with the following 8-bit unsigned integer flags:

- `0`: Empty corridor (passable path).
- `1`: Wall Type 1 (outer border, solid boundaries, and alignment padding).
- `2`: Wall Type 2 (inner procedural maze walls).

#### Usage Example

The following example demonstrates how to initialize the `MazeManager` and generate a procedurally mapped maze within your application:

```cpp
#include "src/maze/MazeManager.h"

// 1. Calculate the active play area dimensions
const int play_width = 240;
const int play_height = 260; // e.g., display_height - ui_header_height
const int wall_thickness = 6;

// 2. Allocate the flat pixel buffer 
uint8_t* gameBoard = new uint8_t[play_width * play_height];

// 3. Instantiate the MazeManager
MazeManager mazeGenerator(play_width, play_height, wall_thickness);

// 4. Generate the maze layout
bool success = mazeGenerator.generate(gameBoard);
if (success) {
    // The gameBoard buffer is now populated with values 0, 1, and 2
    // and is ready for the rendering engine or physics collider.
} else {
    // Generation failed due to invalid dimensions (e.g., width/height too small).
    // The buffer is safely filled with Wall Type 1 as a fallback.
}
```

### Physics Engine

The physics is a shared, game-agnostic 2D simulation under `esp32/src/physics/`. The same `PhysicsEngine` drives both games; each game owns its own `PhysicsBody` (the ball) and supplies a collision environment via the `ICollider` interface.

#### Components

- **`Vec2`** — minimal 2D float vector.
- **`PhysicsBody`** — dynamic ball state: position (`x`, `y`), velocity (`vx`, `vy`) and `radius` (kept smaller than the corridor width for precise movement).
- **`PhysicsParams`** — runtime-tunable values (sensitivity, restitution, EMA alpha, deadzone, max speed, linear damping), defaulted from `config.h`.
- **`PhysicsEngine`** — holds the parameters and EMA filter state and provides the integration and collision-resolution algorithms.
- **`ICollider` / `Contact`** — strategy interface returning the deepest current `Contact` (out-facing `normal`, `penetration` depth, `hit` flag).
- **`BorderCollider`** — keeps the ball inside the axis-aligned play-field rectangle (used by Game 2).

#### Simulation Pipeline

1. **Input conditioning** — `inputAccel(tiltX, tiltY)` applies an EMA low-pass filter, then a deadzone, then sensitivity scaling to turn raw tilt into an acceleration vector. The engine is decoupled from `SensorData`, so the raw sensor values stay untouched for telemetry.
2. **Integration** — `step()` uses semi-implicit Euler (velocity first, then position) and clamps the speed to `maxSpeed`.
3. **Anti-tunneling** — motion is sub-stepped so a small ball cannot pass through a thin wall in a single frame.
4. **Collision response** — on contact only the velocity component normal to the wall is reflected and scaled by `restitution`; the tangential component is preserved, so a ball scraping along a corridor keeps its speed. Energy is conserved except at collisions (no per-frame friction by default).

The method `step()` returns `true` when at least one collision was resolved (maps to telemetry `physics.collision_detected`).

#### Basic Usage

```cpp
#include "src/physics/PhysicsEngine.h"
#include "src/physics/BorderCollider.h"

PhysicsEngine engine;
PhysicsBody ball{ /* x */ 120, /* y */ 140 };
BorderCollider world(play_width, play_height);

// Per 50Hz tick (dt = 0.02s):
SensorData s = sensorManager.read();
Vec2 accel = engine.inputAccel(s.accelerometerX, s.accelerometerY);
bool collided = engine.step(ball, accel, 0.02f, world);
```

### Game Logic & Cookies

The collectible/score logic under `esp32/src/game/` is shared between both games and depends only on the physics `PhysicsBody`, keeping it reusable and unit-testable.

- **`Cookie`** — a single collectible: position, `radius` and an `active` flag (false once eaten until it respawns).
- **`GameState`** — volatile HUD metadata: `status` (`idle` / `running` / `completed`), `cookiesCollected`, `cookiesRemaining`, `currentRound`, `elapsedTimeSec`.
- **`ICookieSpawner`** — strategy that produces a fresh cookie at a valid position, avoiding the ball. Game 1 uses a maze-cell spawner, Game 2 uses `RectCookieSpawner`.
- **`MazeCookieSpawner`** — spawner for the maze game field (1). Spawns a cookie at a random corridor cell center, avoiding the ball. Needs a reference to the `MazeManager` to access the `FreeCells` list.
- **`RectCookieSpawner`** — spawner for the plane game field (2). Spawns a cookie at a random point inside the play-field, kept a margin from the edges and away from the ball.
- **`CookieField`** — keeps a small fixed set of visible cookies (no heap, MCU-friendly). On contact it scores and respawns the eaten cookie via the injected spawner; the round is won once `collected()` reaches `target()`. The game loop queries it via `collected()`, `remaining()` and `finished()`.

#### Basic Usage of RectCookieSpawner and CookieField

```cpp
#include "src/game/CookieField.h"
#include "src/game/RectCookieSpawner.h"

RectCookieSpawner spawner(play_width, play_height, /* cookieRadius */ default_cookie_radius);
CookieField field;
field.start(/* visibleCount */ default_max_visible_cookies, /* target */ default_cookies_count, spawner, ball);

// Per tick, after moving the ball:
field.checkPickup(ball);
if (field.finished()) { /* round complete */ }
```

#### Basic Usage of MazeCookieSpawner

```cpp
#include "src/game/CookieField.h"
#include "src/game/MazeCookieSpawner.h"

// 1. Generate maze (see MazeManager section for setup)
mazeGenerator.generate(gameBoard);

// 2. Setup spawner with free cells from MazeManager
MazeCookieSpawner spawner(&mazeGenerator.getFreeCells(), /* cookieRadius */ default_cookie_radius);

// 3. Initialize and start the cookie field, see same above.
```
