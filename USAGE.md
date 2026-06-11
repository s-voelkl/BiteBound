# BiteBound Usage Guide

## Hardware Setup (ESP32)

Create a `esp32/wifi_mqtt_secrets.h` file based on the provided `esp32/wifi_mqtt_secrets_template.txt` template to store the WiFi and MQTT credentials for the ESP32 device. Fill in the actual values for WiFi and MQTT credentials in the `wifi_mqtt_secrets.h` file. **Do not push actual credentials to version control.**

### Arduino Setup

#### Initial Setup

- Follow the [Getting Started with Arduino](https://docs.arduino.cc/learn/starting-guide/getting-started-arduino/)
    guide to set up the Arduino IDE and configure it for ESP32 development.
    Use an Arduino IDE Version 12.3.7 or later.
- Install the required board managers (see below).
- Install the required libraries (see below).
- Connect the ESP32 to your computer via USB and select the appropriate board and port in the Arduino IDE. The board is named "ESP32-S3-Touch-LCD-1.69" in the board manager.
- In the Arduino IDE, go to `Tools > USB CDC On Boot > Enabled` to enable USB CDC for serial communication (see [ESP32-S3](https://esp32s3.com/getting-started.html))
- Start the [esp32/esp32.ino](esp32/esp32.ino) sketch in the Arduino IDE and upload it to the ESP32.
- Press the reset button ``RST`` on the ESP32 after uploading to start the program.

#### Required Libraries

- ArduinoHttpClient (Arduino)
- ArduinoJson (Benoit Blanchon)
- WiFi (Arduino)
- AUnit (Brian T. Park)
- PubSubClient (Nick O'Leary)
- Waveshare Libraries (Waveshare) from
    [GitHub under ESP32-S3-Touch-LCD-1.69](https://github.com/waveshareteam/ESP32-S3-Touch-LCD-1.69/tree/main/examples/Arduino/libraries). Do not update by Arduino Library Manager!
  - GFX_Library_for_Arduino
  - Mylibrary
  - SensorLib
  - lvgl

#### Required Board Managers

- esp32 by Espressif Systems with Version 3.3.2 for Waveshare compatability.

### Hardware MQTT Mock

For developing and testing the dashboard without the need of the ESP32, a hardware MQTT mock in ``tests/mqtt_mock/esp32_sender_mock.py`` is provided.
It simulates the MQTT communication between the ESP32 and the dashboard (Android/Node-RED) by sending mock telemetry data to the MQTT broker.
The configuration from `config.json` must be adjusted to match the MQTT and authentication settings.
Further information can be found in the ``README.md`` in the ``mqtt_mock`` directory.

## Node-RED Setup

### Local running Node-RED

See: [Running Node-RED locally](https://nodered.org/docs/getting-started/local)

- Change directory: ``cd nodered``
- NPM install: ``npm install`` (``sudo`` might be needed. Installs packages from `package.json`)
- Run: ``node-red --settings settings.js flows.json``
- Open browser: [http://localhost:1880](http://localhost:1880)
- Edit the flow, then deploy for saving the changes to `flows.json`

Verify that ``node-red-contrib-uibuilder`` is installed as a dependency in the NodeRED installed nodes.
Import the project flow from `nodered/flow.json` into the Node-RED editor to set up the dashboard and MQTT communication.

For further information regarding the UI Builder Dashboard creation, see the [UI Builder documentation](https://totallyinformation.github.io/node-red-contrib-uibuilder/#/).

### Docker setup for Node-RED

See: [Running Node-RED with Docker](https://nodered.org/docs/getting-started/docker)
See: [Docker Hub Node-RED image](https://hub.docker.com/r/nodered/node-red/)

- Docker run: ``docker run -it -p 1880:1880 -v node_red_data:/data --name mynodered nodered/node-red``

## Android Setup

tbd

## Documentation

### LaTeX Documentation

The documentation is written in LaTeX and can be found in the `tex/` directory.

### PlantUML Diagrams

For generating diagrams, `PlantUML` is used.
The `.vscode/settings.json` file contains the configuration for the PlantUML extension in Visual Studio Code, which allows you to generate diagrams from the `.puml` files in the `diagrams/src/` directory and output them to the `diagrams/out/` directory.

For exporting, type ``CTRL + SHIFT + P` and select "PlantUML: Export Current Diagram" or "PlantUML: Export All Diagrams" to generate the diagrams in the specified output directory.

Press ``ALT + D`` to open the preview of the current diagram in Visual Studio Code.
