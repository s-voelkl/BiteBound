# NodeRED Dashboard Library Decision

Prompt:

```txt
I am developing an embedded 2D game on an ESP32-S3. The application has two main components with vastly different timing requirements: a high-frequency game loop running at circa 50Hz (handling low-pass filtered IMU input, physics sub-stepping, collision resolution, and display rendering) and a low-frequency network stack running at circa 2Hz (handling blocking MQTT connections, TLS handshakes, and telemetry JSON serialization).

We need to establish our Node-RED dashboard architecture under the `nodered/` directory. The use case requires the dashboard to act as a master operator console that:
1. Displays real-time game telemetry (ball positioning, score states, cookie field configurations) updated every 0.5s.
2. Injects control payloads (START / STOP triggers and runtime variable modifications) into the ESP32 queue via MQTT.

What is the recommended library or architectural approach to build this interface within Node-RED to ensure long-term support, performance, and UI flexibility?
```

Answer with Gemini Flash 3.5 Thinking:

```txt
The recommended approches for building a Node-RED dashboard interface for your ESP32-S3 embedded 2D game are as follows:
- flowfuse/node-red-dashboard (Dashboard 2.0): This is the official, modern ecosystem standard for Node-RED dashboards. It is actively maintained and built on Vue 3 + Vuetify, providing a low-code configuration with code-hybrid templates. It supports native Vue reactivity, which can handle JSON streams instantly, making it suitable for your 0.5s telemetry updates. The modular `ui-template` allows for embedded CSS/Vue components, offering flexibility in UI customization.
- node-red-contrib-uibuilder: This is a long-standing community-driven alternative that is framework agnostic, allowing you to use Vanilla JS, React, or Svelte. It is code-first, meaning you manage front-end build pipelines yourself. It uses dedicated WebSockets for high-performance data streaming and provides absolute freedom in UI design, but requires more effort to set up and maintain.

Framework Evaluation for Your Operator Console

| Metric / Feature | `@flowfuse/node-red-dashboard` (Dashboard 2.0) | `node-red-contrib-uibuilder` |
| --- | --- | --- |
| **LTS Status** | **Active** (The official, modern ecosystem standard) | **Active** (Long-standing community-driven alternative) |
| **Core Architecture** | Vue 3 + Vuetify | Framework Agnostic (Vanilla JS, React, Svelte) |
| **Development Model** | Low-Code configuration with code-hybrid templates | Code-First (You manage front-end build pipelines) |
| **2Hz Telemetry Sync** | Native Vue reactivity handles JSON streams instantly | Uses dedicated WebSockets; highly performant |
| **UI Customization** | Modular `ui-template` allows embedded CSS/Vue components | Absolute freedom, but requires writing all UI scaffolding |

**Verdict:** Choose **Dashboard 2.0**. It provides out-of-the-box UI controls (sliders, buttons, switches) for your runtime control payloads, while offering the flexbility of custom Canvas/SVG nodes for rendering real-time game telemetry.

To support the BiteBound telemetry and control system, you should utilize the Node-RED UI Builder Dashboard (node-red-contrib-uibuilder) rather than the standard Node-RED dashboard nodes.

The primary driver for this architectural decision is that the original, traditional "node-red-dashboard" library is officially deprecated and no longer maintained for modern Node-RED versions. Continuing with the legacy library introduces security liabilities and lack of support for modern Node.js runtimes.

By selecting UI Builder for this use case, you gain several advantages:
- Data-Stream Performance: UI Builder decouples the frontend from the Node-RED backend using optimized WebSockets. This ensures that the 0.5s telemetry snapshots do not cause lag or choke the backend flow orchestrations.
- Bidirectional Control: It cleanly maps frontend events (clicking START/STOP or sliding parameter configurations) to Node-RED messages, which can immediately be routed to your Core 0 MQTT publish nodes.
- Frontend Freedom: You can build a modern, responsive UI using vanilla HTML/JS or lightweight frameworks (like Vue or custom CSS grids) to clearly map out the game state without being restricted by the rigid grid of the deprecated library.
```
