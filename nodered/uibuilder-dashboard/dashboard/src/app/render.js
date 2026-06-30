import { formatUptime, formatElapsedTime, formatBatteryPercentage } from './format.js';

/**
 * Handles DOM manipulation and real-time canvas updates.
 */
export class TelemetryRenderer {
    constructor() {
        this.canvas = document.getElementById('physicsCanvas');
        if (this.canvas) {
            this.ctx = this.canvas.getContext('2d');
        }
    }

    /**
     * Global execution pass updating dashboards based on telemetry.
     */
    update(telemetry, connectionState) {
        this.updateConnectionStatus(connectionState);
        if (!telemetry) return;

        this.updateDeviceSection(telemetry.device);
        this.updateConfigSection(telemetry.config);
        this.updateGameStateSection(telemetry.state);
        this.updateSensorSection(telemetry.sensors);
        this.updatePhysicsMetrics(telemetry.physics);
        this.drawPhysicsCanvas(telemetry);
    }

    /**
     * Updates CSS badges and details of the current MQTT status.
     */
    updateConnectionStatus(state) {
        const el = document.getElementById('connectionState');
        if (!el) return;
        el.textContent = state;
        
        el.className = 'status-badge';
        if (state === 'Connected') {
            el.classList.add('badge-success');
        } else if (state === 'Connecting') {
            el.classList.add('badge-warning');
        } else {
            el.classList.add('badge-danger');
        }
    }

    updateDeviceSection(device) {
        this.setText('deviceClientId', device.clientId);
        this.setText('deviceHardware', device.hardware);
        this.setText('deviceFirmware', 'v' + device.firmwareVersion);
        this.setText('deviceUptime', formatUptime(device.uptimeMs));
        this.setText('deviceWifi', device.wifiSsid);
    }

    updateConfigSection(config) {
        this.setText('configPlayerName', config.playerName);
        this.setText('configGameId', config.gameId === 1 ? 'Labyrinth' : 'Flatland');
        this.setText('configTargetCookies', config.targetCookies);
        this.setText('configWallThickness', config.wallThicknessPx + ' px');
        this.setText('configDimensions', `${config.screenWidth}x${config.screenHeight}`);
    }

    updateGameStateSection(state) {
        this.setText('stateStatus', state.runningStatus.toUpperCase());
        
        const statusEl = document.getElementById('stateStatus');
        if (statusEl) {
            statusEl.className = 'status-text';
            if (state.isRunning) {
                statusEl.className = 'status-text text-success';
            } else if (state.isFinished) {
                statusEl.className = 'status-text text-info';
            } else {
                statusEl.className = 'status-text text-muted';
            }
        }

        this.setText('stateCollected', state.cookiesCollected);
        this.setText('stateRemaining', state.cookiesRemaining);
        this.setText('stateRound', state.currentRound);
        this.setText('stateElapsedTime', formatElapsedTime(state.elapsedTimeSec));
    }

    updateSensorSection(sensors) {
        this.setText('sensorAccelX', sensors.accelX.toFixed(3));
        this.setText('sensorAccelY', sensors.accelY.toFixed(3));
        this.setText('sensorAccelZ', sensors.accelZ.toFixed(3));
        
        this.setText('sensorGyroX', sensors.gyroX.toFixed(1));
        this.setText('sensorGyroY', sensors.gyroY.toFixed(1));
        this.setText('sensorGyroZ', sensors.gyroZ.toFixed(1));

        const batteryPercent = formatBatteryPercentage(sensors.batteryVoltage);
        this.setText('sensorBattery', `${sensors.batteryVoltage.toFixed(2)}V (${batteryPercent}%)`);
        
        const btnEl = document.getElementById('sensorButton');
        if (btnEl) {
            if (sensors.button) {
                btnEl.textContent = 'Pressed';
                btnEl.className = 'btn-indicator active';
            } else {
                btnEl.textContent = 'Released';
                btnEl.className = 'btn-indicator';
            }
        }
    }

    updatePhysicsMetrics(physics) {
        this.setText('physX', physics.ballPosX.toFixed(1));
        this.setText('physY', physics.ballPosY.toFixed(1));
        this.setText('physVelX', physics.velocityX.toFixed(1));
        this.setText('physVelY', physics.velocityY.toFixed(1));
        this.setText('physAccX', physics.accX.toFixed(2));
        this.setText('physAccY', physics.accY.toFixed(2));
        
        const colEl = document.getElementById('physCollision');
        if (colEl) {
            if (physics.collisionDetected) {
                colEl.textContent = "COLLISION!";
                colEl.className = "collision-active";
            } else {
                colEl.textContent = "Clear";
                colEl.className = "";
            }
        }
    }

    /**
     * Renders a real-time recreation of the physical 2D simulator coordinates.
     */
    drawPhysicsCanvas(telemetry) {
        if (!this.ctx || !this.canvas) return;

        const ctx = this.ctx;
        const canvas = this.canvas;
        const physics = telemetry.physics;
        const config = telemetry.config;

        ctx.clearRect(0, 0, canvas.width, canvas.height);

        // Aspect configuration matching standard layout
        const devW = config.screenWidth || 240;
        const devH = config.screenHeight || 280;
        const scaleX = canvas.width / devW;
        const scaleY = canvas.height / devH;

        // Custom palette bindings
        const styles = getComputedStyle(document.body);
        const milkCream = styles.getPropertyValue('--milk-cream') || '#FFF8EC';
        const caramelBrown = styles.getPropertyValue('--caramel-brown') || '#B5722E';
        const berryRed = styles.getPropertyValue('--berry-red') || '#C0492F';
        const chocolateChip = styles.getPropertyValue('--chocolate-chip') || '#5A3A22';
        const honey = styles.getPropertyValue('--honey') || '#F2B84B';

        // Draw background
        ctx.fillStyle = milkCream;
        ctx.fillRect(0, 0, canvas.width, canvas.height);

        // Draw structural play boundaries
        const wt = (config.wallThicknessPx || 10) * scaleX;
        ctx.strokeStyle = caramelBrown;
        ctx.lineWidth = wt;
        ctx.strokeRect(wt / 2, wt / 2, canvas.width - wt, canvas.height - wt);

        // Render cherry/ball body
        const ballX = physics.ballPosX * scaleX;
        const ballY = physics.ballPosY * scaleY;
        const ballRadius = 8 * scaleX;

        ctx.beginPath();
        ctx.arc(ballX, ballY, ballRadius, 0, 2 * Math.PI);
        ctx.fillStyle = physics.collisionDetected ? berryRed : chocolateChip;
        ctx.fill();
        ctx.closePath();

        // Vector directional projection
        ctx.beginPath();
        ctx.moveTo(ballX, ballY);
        ctx.lineTo(
            ballX + (physics.velocityX * 0.1 * scaleX), 
            ballY + (physics.velocityY * 0.1 * scaleY)
        );
        ctx.strokeStyle = honey;
        ctx.lineWidth = 2.5;
        ctx.stroke();
        ctx.closePath();
    }

    setText(id, text) {
        const el = document.getElementById(id);
        if (el) el.textContent = text;
    }
}