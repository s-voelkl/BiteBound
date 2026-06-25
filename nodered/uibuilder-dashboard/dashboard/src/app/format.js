/**
 * @param {number} value
 * @param {number} decimals
 * @returns {string}
 */
export function formatNumber(value, decimals = 2) {
    if (!Number.isFinite(value)) return '0';
    return value.toFixed(decimals);
}

/**
 * @param {number} seconds
 * @returns {string}
 */
export function formatDuration(seconds) {
    const safe = Number.isFinite(seconds) ? Math.max(0, Math.floor(seconds)) : 0;
    const m = Math.floor(safe / 60);
    const s = safe % 60;
    return m > 0 ? `${m}:${String(s).padStart(2, '0')}` : `${s}s`;
}

/**
 * @param {string} status
 * @returns {string}
 */
export function formatStatus(status) {
    if (!status) return 'Idle';
    return status.charAt(0).toUpperCase() + status.slice(1).toLowerCase();
}

/**
 * @param {number} collected
 * @param {number} target
 * @returns {number}
 */
export function progressFraction(collected, target) {
    if (!Number.isFinite(target) || target <= 0) return 0;
    const fraction = collected / target;
    return Math.max(0, Math.min(1, fraction));
}
