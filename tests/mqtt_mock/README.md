# MQTT Mock Test

This Test mocks the MQTT communication between the ESP32 and the dashboard (Android/Node-RED) for testing the dashboard without the need of the ESP32.

## Usage

- Change directory: ``cd tests/mqtt_mock``
- Environment: Python 3.14 with ``python3 -m venv .venv`` and ``source .venv/bin/activate`` (or Windows: ``.venv\Scripts\activate``)
- Install dependencies: ``pip install -r requirements.txt``
- Run: ``python esp32_sender_mock.py``
- Run the dashboard in another terminal
