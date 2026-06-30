import { formatUptime, formatElapsedTime } from './format.js';

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
        if (!telemetry) return;

        this.updateDeviceSection(telemetry.device);
        this.updateGameStateSection(telemetry.state);
        this.updateSensorSection(telemetry.sensors);
        this.drawPhysicsCanvas(telemetry);
    }

    updateDeviceSection(device) {
        this.setText('deviceClientId', device.clientId);
        this.setText('deviceHardware', device.hardware);
        this.setText('deviceFirmware', 'v' + device.firmwareVersion);
        this.setText('deviceUptime', formatUptime(device.uptimeMs));
        this.setText('deviceWifi', device.wifiSsid);
    }

    updateGameStateSection(state) {
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

        // Display raw voltage output without percentage attributes
        this.setText('sensorBattery', `${sensors.batteryVoltage.toFixed(2)} V`);
        
        // Render buttons like other standard sensor variables
        this.setText('sensorButton', sensors.button ? 'Pressed' : 'Released');
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