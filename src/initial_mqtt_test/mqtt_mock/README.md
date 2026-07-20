# MQTT Mock Test

These files are obsolete and only provided for reference. They were used during the first project iteration to test the dashboard without the need of the ESP32.

This Test mocks the MQTT communication between the ESP32 and the dashboard (Android/Node-RED) for testing the dashboard without the need of the ESP32.

## Usage

- Change directory: ``cd initial_mqtt_test/mqtt_mock``
- Environment: Python 3.14 with ``python3 -m venv .venv`` and ``source .venv/bin/activate`` (or Windows: ``.venv\Scripts\activate``)
- Install dependencies: ``pip install -r requirements.txt``
- Run: ``python esp32_sender_mock.py``
- Run the dashboard in another terminal
