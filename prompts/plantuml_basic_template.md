# Basic Template for PlantUML Diagram

Docstrings were partly generated using AI assistance.

Prompt:

```txt
I am developing an embedded 2D game on an ESP32-S3. The application has two main components with vastly different timing requirements: a high-frequency game loop running at circa 50Hz (handling low-pass filtered IMU input, physics sub-stepping, collision resolution, and display rendering) and a low-frequency network stack running at circa 2Hz (handling blocking MQTT connections, TLS handshakes, and telemetry JSON serialization).
I need a high-level architectural skeleton for an ESP32-based IoT game. 

Please provide a basic object-oriented design and PlantUML outline showing the key components (Main, Network, Graphics, Physics, Sensors) and how they should safely interact or share state via FreeRTOS. Keep it foundational so we can refine it further later on. No complex game logic needed yet, just the high-level class layout and structural breakdown.
```

Answer with Gemini Flash 3.5 Thinking:

```txt
Here is a foundational architecture blueprint designed to split workloads cleanly across both ESP32 cores, utilizing a synchronized shared state. This initial layout establishes the core boundaries before introducing complex interface abstractions.

### Core Architecture Breakdown

1. Core 1 (Game Thread): Drives the deterministic execution loop. It polls sensors, steps the physics simulation, and flushes the buffer to the display.
2. Core 0 (Network Thread): Operates asynchronously, dealing with unpredictable I/O latencies from WiFi and MQTT brokers, parsing incoming commands into message queues.
3. Shared Data: A thread-safe data bucket protected by a FreeRTOS Mutex Semaphore to prevent race conditions during telemetry reads and command writes.

### Conceptual Graphics Management
In a dual-core setup, the Graphics Manager on Core 1 typically operates on a non-blocking draw cycle synchronized with the physics timestep (e.g., 50Hz / 20ms). Instead of clearing the entire screen (which induces heavy SPI overhead and lag), it should follow a basic dirty-rectangles or differential drawing pattern:
- Erase previous frame's moving entity positions using background-colored primitives.
- Draw new entity positions.
- Render static UI overlays only on state changes.

---

### Initial PlantUML Skeleton

@startuml BasicGameArchitecture
top to bottom direction

package "Core 0 (Networking & Async I/O)" {
    class WifiManager {
        + connect() : bool
        + isConnected() : bool
    }
    class MqttManager {
        + init() : void
        + publishTelemetry(payload : String) : bool
        + handleIncomingCommand(topic : char*, payload : byte*) : void
    }
}

package "Shared Space (IPC)" {
    struct SharedSystemState {
        + ballX : float
        + ballY : float
        + score : int
        + systemStatus : int
    }
    class MutexWrapper {
        + lock() : void
        + unlock() : void
    }
}

package "Core 1 (Game Engine & Peripherals)" {
    class MainApp {
        + setup() : void
        + gameLoop() : void
    }
    class SensorManager {
        + readIMU() : void
    }
    class PhysicsEngine {
        + updatePosition(ax : float, ay : float) : void
    }
    class GraphicsManager {
        + initDisplay() : void
        + renderFrame(state : SharedSystemState) : void
    }
}

' Interactions
MainApp --> SensorManager : "Polls data"
MainApp --> PhysicsEngine : "Steps simulation"
MainApp --> GraphicsManager : "Triggers redraw"
MainApp ..> MutexWrapper : "Protects writes"
MainApp ..> SharedSystemState : "Updates state"

MqttManager ..> WifiManager : "Depends on"
MqttManager ..> MutexWrapper : "Protects reads"
MqttManager ..> SharedSystemState : "Reads for telemetry"

@enduml
```
