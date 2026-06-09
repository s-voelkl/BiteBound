# MAUC

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
