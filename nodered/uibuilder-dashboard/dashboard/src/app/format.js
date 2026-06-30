/**
 * Utility Formatter Functions for UI Presentation
 */

/**
 * Converts milliseconds to hh:mm:ss format
 */
export function formatUptime(ms) {
    const seconds = Math.floor(ms / 1000);
    const hrs = Math.floor(seconds / 3600);
    const mins = Math.floor((seconds % 3600) / 60);
    const secs = seconds % 60;
    return `${hrs.toString().padStart(2, '0')}:${mins.toString().padStart(2, '0')}:${secs.toString().padStart(2, '0')}`;
}

/**
 * Formats stopwatch seconds to mm:ss.ms
 */
export function formatElapsedTime(sec) {
    const m = Math.floor(sec / 60);
    const s = Math.floor(sec % 60);
    const ms = Math.floor((sec % 1) * 1000);
    return `${m.toString().padStart(2, '0')}:${s.toString().padStart(2, '0')}.${ms.toString().padStart(3, '0')}`;
}

/**
 * Estimates battery percentage based on a standard 3.3V to 4.2V scale.
 */
export function formatBatteryPercentage(voltage) {
    const minV = 3.3;
    const maxV = 4.2;
    if (voltage <= 0) return 0;
    if (voltage < minV) return 0;
    if (voltage > maxV) return 100;
    return Math.round(((voltage - minV) / (maxV - minV)) * 100);
}

/**
 * Returns ISO-8601 UTC timestamp string
 */
export function isoNow() {
    return new Date().toISOString();
}

/**
 * Generates RFC4122 v4 UUID
 */
export function generateUUID() {
    return 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, function(c) {
        const r = Math.random() * 16 | 0, v = c === 'x' ? r : (r & 0x3 | 0x8);
        return v.toString(16);
    });
}