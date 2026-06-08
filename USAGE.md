# Usage instructions for the project

## Hardware Setup (ESP32)

tbd

## Node-RED Setup

### Local running Node-RED

See: [Running Node-RED locally](https://nodered.org/docs/getting-started/local)

- NPM install: ``sudo npm install -g node-red``
- Change directory: ``cd nodered``
- Run: ``node-red --settings settings.js flows.json``
- Open browser: [http://localhost:1880](http://localhost:1880)
- Edit the flow, then deploy for saving the changes to `flows.json`

Import the project flow from `nodered/flow.json` into the Node-RED editor to set up the dashboard and MQTT communication.

### Docker setup for Node-RED

See: [Running Node-RED with Docker](https://nodered.org/docs/getting-started/docker)
See: [Docker Hub Node-RED image](https://hub.docker.com/r/nodered/node-red/)

- Docker run: ``docker run -it -p 1880:1880 -v node_red_data:/data --name mynodered nodered/node-red``

## Android Setup

tbd
