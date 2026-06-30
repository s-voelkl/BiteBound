# BiteBound Node-RED UI Builder Dashboard

This directory houses the source files for the BiteBound web-based Node-RED UI Builder dashboard.
It mirrors the functionality, constraints, and visual styling of the native Kotlin-based Android application.

The dashboard receives real-time telemetry streams from the ESP32 (via a Node-RED WebSocket bridge to the HiveMQ MQTT broker) and publishes commands to control game play, modify physics parameters, and adjust game rules.

---

This documentation overview was partly AI generated, to provide a concise summary for other developers.

## File Overview

```txt
src/
├── index.html               # Main Single-Page Application (SPA) structure
├── index.css                # Layout styling and UI overrides
├── index.js                 # Event handler orchestrator and uibuilder bridge
└── app/
    ├── constants.js         # Game limits, physics thresholds, and ball presets
    ├── types.js             # Struct fallbacks to guarantee robust data handling
    ├── format.js            # Time formatting, battery calculations, and UUID helpers
    ├── telemetry.js         # Telemetry parsing layer
    ├── command.js           # JSON command constructors (start, stop, resume)
    ├── render.js            # Canvas painter and dynamic DOM manipulator
    └── theme.css            # Light and dark mode "cookie" palette declarations
assets/
└── logo.png             # Project visual branding asset
```

### File Descriptions

* **`index.html`**: Defines the structural layout of the dashboard grid, utilizing a hidden telemetry-bind system to reveal controls only once the physical hardware establishes a handshake.
* **`index.css`**: Manages responsiveness, 3-column desktop layouts, warm theme palettes, and interactive transitions.
* **`index.js`**: Integrates modules, binds form inputs, runs clamping logic as users type, handles the New Game dialog workflow, and routes WebSocket data.
* **`app/constants.js`**: Centralizes game configurations to prevent hardcoded numbers in different parts of the code. Keeps physics multipliers in sync with the ESP32.
* **`app/types.js`**: Replicates the Kotlin parsing logic. If the physical hardware drops a sensor key, this layer populates a default value so the UI doesn't crash.
* **`app/format.js`**: Keeps user-facing metric strings clean (e.g. converting uptime ticks to `hh:mm:ss`).
* **`app/command.js`**: Structures outgoing control commands to match the exact JSON schema required by the microcontroller's command parser.
* **`app/telemetry.js`**: Sanitizes and standardizes incoming websocket updates.
* **`app/render.js`**: Draws the physical game state (boundaries, coordinates, and velocity vectors) on an HTML5 canvas, manages indicator states, and updates circular score charts.
* **`app/theme.css`**: Holds the CSS variables defining the warm baking-palette color schemes, adjusting styling according to local system preferences.

---

### Core Design Updates

* **Zero-Navigation Flow**: Removed the connection credential screens to load attributes directly from backend Node-RED definitions and local storage persistence.
* **Compact Viewport Optimization**: Designed a 3-column viewport grid on desktop (utilizing `display: contents` combined with column constraints) that avoids vertical scrolling, organizing layout details into side-by-side elements.
* **Responsive Flex Scoreboards**: Structured Cookie Scores using horizontal flexboxes, placing circular SVG progress rings on the left and stats sequentially on the right.
* **Improved Form Styling**: Input fields are redesigned to fit the bakery theme with warm color palettes and standard borders. Interactive transitions on disabled states avoid jarring layout shifts.
* **Harmonized Diagnostics**: Grouped sensor battery readings and system buttons into a standardized tile template for easier visual scanning.

---

## Core Concepts Used

### 1. Direct Telemetry Handshake

Rather than requiring a manual connection step, the dashboard initializes into a "Warming up" state. It automatically transitions to the active gameplay view as soon as the first valid telemetry packet is received via the WebSocket bridge.

### 2. Compact Viewport Optimization

Designed to eliminate vertical scrolling on desktop browsers, the dashboard utilizes a 3-column CSS grid. By using `display: contents` on the telemetry container, the internal layout columns are promoted directly to the primary grid fragments, ensuring side-by-side organization of the score, board, and controls.

### 3. Defensive JSON Deserialization

Due to the possibility of UDP packet dropping or half-transmitted serial data on resource-constrained microcontrollers, the telemetry parsing uses a safe fallback model. Every incoming variable is verified against expected data types and defaulted on failure before rendering.

### 4. Dynamic SVG Progress Rings

Rather than using heavy charting dependencies, the "Cookie Score" utilizes a lightweight SVG progress path. The client calculates the collected/target ratio and sets the circle's `stroke-dashoffset` programmatically to animate the filled segment.

### 5. Vector Canvas Projection

The `MiniGameBoard` uses a 2D HTML5 canvas context. Incoming $X/Y$ coordinates are scaled from the physical screen resolution ($240 \times 280$) to match the fluid DOM boundaries. A scaled line represents the ball’s current velocity vector to help diagnose physical behavior.

### 6. Input Sanitization on Type

Parameters like cookie limits or wall thickness are parsed during inputs to strip out non-numeric characters and constrain variables instantly (e.g. capping cookies at 20).
