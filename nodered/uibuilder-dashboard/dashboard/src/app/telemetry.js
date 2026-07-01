import { 
    createDefaultDevice, 
    createDefaultGameConfig, 
    createDefaultGameState, 
    createDefaultPhysics, 
    createDefaultSensors 
} from './types.js';

/**
 * Parses and manages raw telemetry packets streaming from Node-RED over WebSockets.
 */
export class TelemetryParser {
    /**
     * Safely parses incoming JSON telemetry payload into standard objects.
     * @param {string|object} raw - The raw string or JSON object from the MQTT subscription.
     * @returns {object|null} Structured telemetry data, or null on execution error.
     */
    static parse(raw) {
        let root = {};
        if (typeof raw === 'string') {
            try {
                root = JSON.parse(raw);
            } catch (e) {
                console.error("Telemetry parsing error", e);
                return null;
            }
        } else if (typeof raw === 'object' && raw !== null) {
            root = raw;
        }

        return {
            device: createDefaultDevice(root.device),
            config: createDefaultGameConfig(root.config),
            state: createDefaultGameState(root.state),
            physics: createDefaultPhysics(root.physics),
            sensors: createDefaultSensors(root.sensors),
            receivedAtMillis: Date.now()
        };
    }
}