#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

// ==========================================
//              CONFIGURATION
// ==========================================

// ===== Pins =====
#define SERVO1 13  // Front Left
#define SERVO2 12  // Front Right
#define SERVO3 14  // Rear Left
#define SERVO4 27  // Rear Right

#define TRIG 5
#define ECHO 18

// ===== WiFi Settings =====
const char* ssid = "SPYDER_ROBOT";
const char* password = "12345678";

// ===== Server =====
WebServer server(80);

// ===== Servo Objects =====
Servo s1, s2, s3, s4;

// ===== State Variables =====
String robotState = "STOPPED"; // STARTED, STOPPED
bool moveForward = false;
bool moveBackward = false;
unsigned long lastMoveTime = 0;
int moveStep = 0;

// ===== Sensor Variables =====
long distanceFront = 0;
unsigned long lastSensorTime = 0;

// ==========================================
//              WEB RESOURCES
// ==========================================

const char html_content[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Spider Robot Control Panel</title>
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.0/css/all.min.css">
    <style>
* {
    margin: 0;
    padding: 0;
    box-sizing: border-box;
    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
}

body {
    background: linear-gradient(135deg, #0c0c0c, #1a1a2e);
    color: #f0f0f0;
    min-height: 100vh;
    padding: 20px;
    line-height: 1.6;
}

.container {
    max-width: 1200px;
    margin: 0 auto;
}

header {
    text-align: center;
    padding: 20px 0 40px;
    border-bottom: 1px solid #333;
    margin-bottom: 30px;
}

.logo {
    display: flex;
    align-items: center;
    justify-content: center;
    gap: 15px;
    margin-bottom: 15px;
    flex-wrap: wrap;
}

.logo i {
    color: #00d9ff;
    font-size: 2.8rem;
}

h1 {
    font-size: 2.5rem;
    background: linear-gradient(90deg, #00d9ff, #0088ff);
    -webkit-background-clip: text;
    background-clip: text;
    color: transparent;
    text-shadow: 0 2px 5px rgba(0, 0, 0, 0.3);
}

.subtitle {
    color: #aaa;
    font-size: 1.1rem;
    margin-top: 5px;
    max-width: 600px;
    margin-left: auto;
    margin-right: auto;
}

.status-bar {
    display: flex;
    justify-content: space-between;
    background: rgba(20, 20, 30, 0.8);
    padding: 15px;
    border-radius: 10px;
    margin-bottom: 30px;
    border: 1px solid #333;
    flex-wrap: wrap;
    gap: 10px;
}

.status-item {
    display: flex;
    align-items: center;
    gap: 10px;
    padding: 5px 10px;
}

.status-indicator {
    width: 12px;
    height: 12px;
    border-radius: 50%;
    background: #00ff00;
    box-shadow: 0 0 10px #00ff00;
}

.status-indicator.online {
    background: #00ff00;
    box-shadow: 0 0 10px #00ff00;
}

.status-indicator.offline {
    background: #ff0000;
    box-shadow: 0 0 10px #ff0000;
}

.status-indicator.connecting {
    background: #ff9900;
    box-shadow: 0 0 10px #ff9900;
    animation: pulse 1.5s infinite;
}

.dashboard {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(350px, 1fr));
    gap: 25px;
    margin-bottom: 30px;
}

.card {
    background: rgba(30, 30, 45, 0.8);
    border-radius: 15px;
    padding: 25px;
    border: 1px solid #333;
    box-shadow: 0 10px 20px rgba(0, 0, 0, 0.3);
    transition: transform 0.3s, box-shadow 0.3s;
}

.card:hover {
    transform: translateY(-5px);
    box-shadow: 0 15px 30px rgba(0, 0, 0, 0.4);
}

.card-title {
    display: flex;
    align-items: center;
    gap: 15px;
    margin-bottom: 20px;
    padding-bottom: 15px;
    border-bottom: 1px solid #444;
}

.card-title i {
    font-size: 1.8rem;
    color: #00d9ff;
}

h2 {
    font-size: 1.6rem;
}

.sensor-data {
    display: flex;
    flex-direction: column;
    gap: 15px;
}

.sensor-item {
    display: flex;
    justify-content: space-between;
    align-items: center;
    padding: 15px;
    background: rgba(0, 0, 0, 0.3);
    border-radius: 10px;
    border-left: 5px solid #00d9ff;
    transition: background 0.3s;
}

.sensor-item:hover {
    background: rgba(0, 0, 0, 0.5);
}

.sensor-value {
    font-size: 2rem;
    font-weight: bold;
    color: #00d9ff;
}

.sensor-unit {
    color: #aaa;
    font-size: 1rem;
    margin-left: 5px;
}

.servo-controls {
    display: grid;
    grid-template-columns: repeat(2, 1fr);
    gap: 20px;
}

.servo {
    background: rgba(0, 0, 0, 0.3);
    border-radius: 10px;
    padding: 20px;
    text-align: center;
    transition: background 0.3s;
}

.servo:hover {
    background: rgba(0, 0, 0, 0.5);
}

.servo-name {
    display: flex;
    align-items: center;
    justify-content: center;
    gap: 10px;
    margin-bottom: 15px;
    color: #00d9ff;
}

.slider-container {
    margin: 20px 0;
}

.slider {
    width: 100%;
    height: 10px;
    -webkit-appearance: none;
    appearance: none;
    background: linear-gradient(90deg, #333, #00d9ff);
    border-radius: 5px;
    outline: none;
    cursor: pointer;
}

.slider::-webkit-slider-thumb {
    -webkit-appearance: none;
    appearance: none;
    width: 25px;
    height: 25px;
    border-radius: 50%;
    background: #00d9ff;
    cursor: pointer;
    box-shadow: 0 0 10px rgba(0, 217, 255, 0.7);
    border: 2px solid #fff;
    transition: transform 0.2s;
}

.slider::-webkit-slider-thumb:hover {
    transform: scale(1.1);
}

.slider-value {
    font-size: 1.5rem;
    font-weight: bold;
    color: #00d9ff;
    margin-top: 10px;
}

.components-list {
    display: grid;
    grid-template-columns: repeat(auto-fill, minmax(250px, 1fr));
    gap: 15px;
}

.component-item {
    display: flex;
    align-items: center;
    gap: 15px;
    padding: 15px;
    background: rgba(0, 0, 0, 0.3);
    border-radius: 10px;
    border-left: 5px solid;
    transition: transform 0.3s, background 0.3s;
}

.component-item:hover {
    transform: translateY(-3px);
    background: rgba(0, 0, 0, 0.5);
}

.component-item.esp32 {
    border-left-color: #9c27b0;
}

.component-item.ultrasonic {
    border-left-color: #4caf50;
}

.component-item.servo-item {
    border-left-color: #2196f3;
}

.component-item.smoke {
    border-left-color: #ff9800;
}

.component-icon {
    font-size: 2rem;
}

.esp32 .component-icon {
    color: #9c27b0;
}

.ultrasonic .component-icon {
    color: #4caf50;
}

.servo-item .component-icon {
    color: #2196f3;
}

.smoke .component-icon {
    color: #ff9800;
}

.controls {
    display: flex;
    justify-content: center;
    gap: 20px;
    margin-top: 30px;
    flex-wrap: wrap;
}

.btn {
    padding: 15px 30px;
    border: none;
    border-radius: 50px;
    font-size: 1.1rem;
    font-weight: bold;
    cursor: pointer;
    display: flex;
    align-items: center;
    gap: 10px;
    transition: all 0.3s;
    min-width: 180px;
    justify-content: center;
}

.btn-primary {
    background: linear-gradient(90deg, #0088ff, #00d9ff);
    color: white;
}

.btn-secondary {
    background: rgba(255, 255, 255, 0.1);
    color: #ddd;
    border: 1px solid #555;
}

.btn:hover {
    transform: translateY(-3px);
    box-shadow: 0 10px 20px rgba(0, 0, 0, 0.3);
}

.btn:active {
    transform: translateY(-1px);
}

.btn-primary:hover {
    background: linear-gradient(90deg, #00d9ff, #0088ff);
}

.btn-secondary:hover {
    background: rgba(255, 255, 255, 0.2);
}

footer {
    text-align: center;
    margin-top: 50px;
    padding: 20px;
    color: #777;
    border-top: 1px solid #333;
    font-size: 0.9rem;
}

.copyright {
    margin-top: 10px;
    font-size: 0.8rem;
    color: #555;
}

.system-info {
    margin-top: 30px;
}

.info-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
    gap: 20px;
}

.info-item {
    display: flex;
    justify-content: space-between;
    padding: 15px;
    background: rgba(0, 0, 0, 0.3);
    border-radius: 8px;
    border-left: 3px solid #00d9ff;
}

.info-label {
    font-weight: bold;
    color: #aaa;
}

.info-value {
    color: #00d9ff;
    font-weight: bold;
}

.warning {
    color: #ff9800;
    animation: pulse 2s infinite;
}

.danger {
    color: #ff0000;
    animation: pulse 1s infinite;
}

@keyframes pulse {
    0% { opacity: 1; }
    50% { opacity: 0.7; }
    100% { opacity: 1; }
}

@media (max-width: 768px) {
    .dashboard {
        grid-template-columns: 1fr;
    }
    
    .servo-controls {
        grid-template-columns: 1fr;
    }
    
    h1 {
        font-size: 2rem;
    }
    
    .status-bar {
        flex-direction: column;
        align-items: flex-start;
    }
    
    .btn {
        min-width: 150px;
        padding: 12px 20px;
    }
    
    .info-grid {
        grid-template-columns: 1fr;
    }
}

@media (max-width: 480px) {
    .logo {
        flex-direction: column;
        gap: 10px;
    }
    
    .controls {
        flex-direction: column;
        align-items: center;
    }
    
    .btn {
        width: 100%;
        max-width: 300px;
    }
    
    .components-list {
        grid-template-columns: 1fr;
    }
}
    </style>
    <link rel="icon" type="image/x-icon" href="data:image/svg+xml,<svg xmlns=%22http://www.w3.org/2000/svg%22 viewBox=%220 0 100 100%22><text y=%22.9em%22 font-size=%2290%22>🕷️</text></svg>">
</head>
<body>
    <div class="container">
        <header>
            <div class="logo">
                <i class="fas fa-spider"></i>
                <h1>Spider Robot Control Panel</h1>
            </div>
            <p class="subtitle">ESP32-based robot with ultrasonic sensors, servo motors, and smoke detection</p>
        </header>

        <div class="status-bar">
            <div class="status-item">
                <div class="status-indicator" id="esp32-status"></div>
                <span>ESP32: <span id="connection-status">Connected</span></span>
            </div>
            <div class="status-item">
                <div class="status-indicator online"></div>
                <span>Wi-Fi: <span id="wifi-status">Robot_Network</span></span>
            </div>
            <div class="status-item">
                <div class="status-indicator online"></div>
                <span>Battery: <span id="battery-level">87%</span></span>
            </div>
            <div class="status-item">
                <div class="status-indicator online"></div>
                <span>Last Update: <span id="last-update">Just now</span></span>
            </div>
        </div>

        <div class="dashboard">
            <div class="card">
                <div class="card-title">
                    <i class="fas fa-satellite-dish"></i>
                    <h2>Sensor Data</h2>
                </div>
                <div class="sensor-data">
                    <div class="sensor-item">
                        <div>
                            <h3>Ultrasonic Sensor 1</h3>
                            <p>Front distance measurement</p>
                        </div>
                        <div class="sensor-value" id="ultrasonic1">--<span class="sensor-unit">cm</span></div>
                    </div>
                    <div class="sensor-item">
                        <div>
                            <h3>Ultrasonic Sensor 2</h3>
                            <p>Rear distance measurement</p>
                        </div>
                        <div class="sensor-value" id="ultrasonic2">--<span class="sensor-unit">cm</span></div>
                    </div>
                    <div class="sensor-item">
                        <div>
                            <h3>Smoke Sensor</h3>
                            <p>Air quality detection</p>
                        </div>
                        <div class="sensor-value" id="smoke-value">0<span class="sensor-unit">ppm</span></div>
                    </div>
                </div>
            </div>

            <div class="card">
                <div class="card-title">
                    <i class="fas fa-cogs"></i>
                    <h2>Servo Motor Controls</h2>
                </div>
                <div class="servo-controls">
                    <div class="servo">
                        <div class="servo-name">
                            <i class="fas fa-robot"></i>
                            <h3>Front Left Leg</h3>
                        </div>
                        <div class="slider-container">
                            <input type="range" min="0" max="180" value="90" class="slider" id="servo1">
                            <div class="slider-value" id="servo1-value">90°</div>
                        </div>
                    </div>
                    <div class="servo">
                        <div class="servo-name">
                            <i class="fas fa-robot"></i>
                            <h3>Front Right Leg</h3>
                        </div>
                        <div class="slider-container">
                            <input type="range" min="0" max="180" value="90" class="slider" id="servo2">
                            <div class="slider-value" id="servo2-value">90°</div>
                        </div>
                    </div>
                    <div class="servo">
                        <div class="servo-name">
                            <i class="fas fa-robot"></i>
                            <h3>Rear Left Leg</h3>
                        </div>
                        <div class="slider-container">
                            <input type="range" min="0" max="180" value="90" class="slider" id="servo3">
                            <div class="slider-value" id="servo3-value">90°</div>
                        </div>
                    </div>
                    <div class="servo">
                        <div class="servo-name">
                            <i class="fas fa-robot"></i>
                            <h3>Rear Right Leg</h3>
                        </div>
                        <div class="slider-container">
                            <input type="range" min="0" max="180" value="90" class="slider" id="servo4">
                            <div class="slider-value" id="servo4-value">90°</div>
                        </div>
                    </div>
                </div>
            </div>

            <div class="card">
                <div class="card-title">
                    <i class="fas fa-microchip"></i>
                    <h2>Hardware Components</h2>
                </div>
                <div class="components-list">
                    <div class="component-item esp32">
                        <i class="fas fa-microchip component-icon"></i>
                        <div>
                            <h3>ESP32 Microcontroller</h3>
                            <p>Dual-core 240MHz, WiFi+Bluetooth</p>
                        </div>
                    </div>
                    <div class="component-item ultrasonic">
                        <i class="fas fa-wave-square component-icon"></i>
                        <div>
                            <h3>Ultrasonic Sensors (x2)</h3>
                            <p>HC-SR04, 2cm-400cm range</p>
                        </div>
                    </div>
                    <div class="component-item servo-item">
                        <i class="fas fa-cog component-icon"></i>
                        <div>
                            <h3>Servo Motors (x4)</h3>
                            <p>SG90, 180° rotation</p>
                        </div>
                    </div>
                    <div class="component-item smoke">
                        <i class="fas fa-smog component-icon"></i>
                        <div>
                            <h3>Smoke Sensor</h3>
                            <p>MQ-2, LPG/Propane detection</p>
                        </div>
                    </div>
                </div>
            </div>
        </div>

        <div class="controls">
            <button class="btn btn-primary" id="start-btn">
                <i class="fas fa-play"></i> Start Robot
            </button>
            <button class="btn btn-secondary" id="stop-btn">
                <i class="fas fa-stop"></i> Stop Robot
            </button>
            <button class="btn btn-secondary" id="calibrate-btn">
                <i class="fas fa-compass"></i> Calibrate Sensors
            </button>
            <button class="btn btn-secondary" id="reset-btn">
                <i class="fas fa-redo"></i> Reset Position
            </button>
            <button class="btn btn-secondary" id="connect-btn">
                <i class="fas fa-plug"></i> Connect to ESP32
            </button>
        </div>

        <div class="card system-info">
            <div class="card-title">
                <i class="fas fa-info-circle"></i>
                <h2>System Information</h2>
            </div>
            <div class="info-grid">
                <div class="info-item">
                    <span class="info-label">ESP32 IP Address:</span>
                    <span class="info-value" id="ip-address">192.168.4.1</span>
                </div>
                <div class="info-item">
                    <span class="info-label">Web Server Port:</span>
                    <span class="info-value" id="server-port">80</span>
                </div>
                <div class="info-item">
                    <span class="info-label">Uptime:</span>
                    <span class="info-value" id="uptime">0h 0m 0s</span>
                </div>
                <div class="info-item">
                    <span class="info-label">Signal Strength:</span>
                    <span class="info-value" id="signal-strength">-45 dBm</span>
                </div>
            </div>
        </div>

        <footer>
            <p>Spider Robot Project | ESP32 Web Interface | Hardware Components Monitoring System</p>
            <p>Data updates every 2 seconds | Connection Status: <span id="connection-status-footer">Stable</span></p>
            <p class="copyright">© 2025 Spider Robot Project. All rights reserved.</p>
        </footer>
    </div>

    <script>
    // DOM Elements
const ultrasonic1El = document.getElementById('ultrasonic1');
const ultrasonic2El = document.getElementById('ultrasonic2');
const smokeValueEl = document.getElementById('smoke-value');
const lastUpdateEl = document.getElementById('last-update');
const connectionStatusEl = document.getElementById('connection-status');
const esp32StatusIndicator = document.getElementById('esp32-status');
const batteryLevelEl = document.getElementById('battery-level');
const wifiStatusEl = document.getElementById('wifi-status');
const connectionStatusFooterEl = document.getElementById('connection-status-footer');
const ipAddressEl = document.getElementById('ip-address');
const serverPortEl = document.getElementById('server-port');
const uptimeEl = document.getElementById('uptime');
const signalStrengthEl = document.getElementById('signal-strength');

// Servo elements
const servoSliders = document.querySelectorAll('.slider');
const servoValueDisplays = {
    servo1: document.getElementById('servo1-value'),
    servo2: document.getElementById('servo2-value'),
    servo3: document.getElementById('servo3-value'),
    servo4: document.getElementById('servo4-value')
};

// Button elements
const startBtn = document.getElementById('start-btn');
const stopBtn = document.getElementById('stop-btn');
const calibrateBtn = document.getElementById('calibrate-btn');
const resetBtn = document.getElementById('reset-btn');
const connectBtn = document.getElementById('connect-btn');

// Configuration
const config = {
    updateInterval: 1000, 
    simulateData: false,   // Disable simulation to use real ESP32 data
    esp32IP: '', // Empty string means 'relative to current page'
    serverPort: 80
};

// State variables
let isConnected = false;
let updateIntervalId = null;
let robotActive = false;
let uptimeSeconds = 0;

// Initialize the application
function init() {
    console.log('Spider Robot Control Panel Initializing...');
    
    // Set initial UI state
    // Assume connected since we are serving this FROM the ESP32
    updateConnectionStatus(true);
    
    // Initialize servo sliders
    initServoSliders();
    
    // Set up event listeners
    setupEventListeners();
    
    // Start data updates
    startDataUpdates();
    
    // Start uptime counter
    startUptimeCounter();
}

// Initialize servo slider controls
function initServoSliders() {
    servoSliders.forEach(slider => {
        // Set initial value display
        const valueId = slider.id + '-value';
        if (servoValueDisplays[slider.id]) {
            servoValueDisplays[slider.id].textContent = slider.value + '°';
        }
        
        // Update value when slider moves
        slider.addEventListener('input', function() {
            if (servoValueDisplays[this.id]) {
                servoValueDisplays[this.id].textContent = this.value + '°';
            }
            
            // Send servo position to ESP32 (simulated or real)
            sendServoPosition(this.id, this.value);
        });
    });
}

// Set up event listeners for buttons
function setupEventListeners() {
    // Start Robot button
    startBtn.addEventListener('click', function() {
        if (!isConnected) {
            showMessage('Error: Not connected to ESP32', 'error');
            return;
        }
        
        robotActive = true;
        showMessage('Robot movement started!', 'success');
        startBtn.disabled = true;
        stopBtn.disabled = false;
        
        // In real implementation, send command to ESP32
        sendCommand('START_ROBOT');
    });
    
    // Stop Robot button
    stopBtn.addEventListener('click', function() {
        robotActive = false;
        showMessage('Robot movement stopped!', 'info');
        startBtn.disabled = false;
        stopBtn.disabled = true;
        
        // In real implementation, send command to ESP32
        sendCommand('STOP_ROBOT');
    });
    
    // Calibrate Sensors button
    calibrateBtn.addEventListener('click', function() {
        if (!isConnected) {
            showMessage('Error: Not connected to ESP32', 'error');
            return;
        }
        
        showMessage('Calibrating sensors...', 'info');
        
        // In real implementation, send command to ESP32
        sendCommand('CALIBRATE_SENSORS');
    });
    
    // Reset Position button
    resetBtn.addEventListener('click', function() {
        if (!isConnected) {
            showMessage('Error: Not connected to ESP32', 'error');
            return;
        }
        
        // Reset all servos to 90°
        servoSliders.forEach(slider => {
            slider.value = 90;
            if (servoValueDisplays[slider.id]) {
                servoValueDisplays[slider.id].textContent = '90°';
            }
            
            // Send reset position to ESP32
            sendServoPosition(slider.id, 90);
        });
        
        showMessage('All servos reset to default position (90°)', 'success');
    });
    
    // Connect to ESP32 button
    connectBtn.addEventListener('click', toggleConnection);
}

// Toggle connection to ESP32
function toggleConnection() {
    if (isConnected) {
        disconnectFromESP32();
    } else {
        connectToESP32();
    }
}

// Connect to ESP32 (simulated)
function connectToESP32() {
    console.log('Connecting to ESP32...');
    
    // Update UI to show connecting state
    esp32StatusIndicator.className = 'status-indicator connecting';
    connectionStatusEl.textContent = 'Connecting...';
    connectBtn.disabled = true;
    connectBtn.innerHTML = '<i class="fas fa-spinner fa-spin"></i> Connecting...';
    
    // Simulate connection delay
    setTimeout(() => {
        isConnected = true;
        esp32StatusIndicator.className = 'status-indicator online';
        connectionStatusEl.textContent = 'Connected';
        connectionStatusFooterEl.textContent = 'Connected to ESP32';
        
        connectBtn.disabled = false;
        connectBtn.innerHTML = '<i class="fas fa-plug"></i> Disconnect from ESP32';
        connectBtn.classList.add('btn-primary');
        connectBtn.classList.remove('btn-secondary');
        
        showMessage('Successfully connected to ESP32!', 'success');
        
        // Enable robot controls
        startBtn.disabled = false;
        stopBtn.disabled = true;
        calibrateBtn.disabled = false;
        resetBtn.disabled = false;
        
    }, 1500);
}

// Disconnect from ESP32
function disconnectFromESP32() {
    console.log('Disconnecting from ESP32...');
    
    isConnected = false;
    robotActive = false;
    
    // Update UI
    esp32StatusIndicator.className = 'status-indicator offline';
    connectionStatusEl.textContent = 'Disconnected';
    connectionStatusFooterEl.textContent = 'Disconnected';
    
    connectBtn.innerHTML = '<i class="fas fa-plug"></i> Connect to ESP32';
    connectBtn.classList.remove('btn-primary');
    connectBtn.classList.add('btn-secondary');
    
    // Disable robot controls
    startBtn.disabled = true;
    stopBtn.disabled = true;
    calibrateBtn.disabled = true;
    resetBtn.disabled = true;
    
    showMessage('Disconnected from ESP32', 'info');
}

// Update sensor data
function updateSensorData() {
    if (!isConnected && !config.simulateData) {
        // Even if not "connected" via the button state, we try to fetch if we are serving from the ESP32
        // But for safety let's respect the flag or just force it if we are confident.
        // Actually, let's allow it.
    }
    
    // Generate simulated sensor data
    if (config.simulateData) {
        // ... (removed simulation code for brevity, it is not needed) ...
    } else {
        // In real implementation, fetch data from ESP32 API
        fetchDataFromESP32();
    }
    
    // Update timestamp
    const now = new Date();
    lastUpdateEl.textContent = 
        `${now.getHours().toString().padStart(2, '0')}:${now.getMinutes().toString().padStart(2, '0')}:${now.getSeconds().toString().padStart(2, '0')}`;
}

// Start data updates
function startDataUpdates() {
    // Initial update
    updateSensorData();
    
    // Set up interval for updates
    updateIntervalId = setInterval(updateSensorData, config.updateInterval);
}

// Start uptime counter
function startUptimeCounter() {
    setInterval(() => {
        uptimeSeconds++;
        
        const hours = Math.floor(uptimeSeconds / 3600);
        const minutes = Math.floor((uptimeSeconds % 3600) / 60);
        const seconds = uptimeSeconds % 60;
        
        uptimeEl.textContent = `${hours}h ${minutes}m ${seconds}s`;
    }, 1000);
}

// Send servo position to ESP32
function sendServoPosition(servoId, position) {
    if (!isConnected) {
        // console.log(`Simulated: Setting ${servoId} to ${position} degrees`);
        // return;
    }
    
    console.log(`Sending to ESP32: ${servoId} = ${position}°`);
    
    // In real implementation, send HTTP request to ESP32
    // Map IDs: servo1->13, servo2->12, servo3->14, servo4->27
    let pin = 0;
    if(servoId == 'servo1') pin = 13;
    if(servoId == 'servo2') pin = 12;
    if(servoId == 'servo3') pin = 14;
    if(servoId == 'servo4') pin = 27;
    
    fetch(`/servo/${pin}?position=${position}`);
}

// Send command to ESP32
function sendCommand(command) {
    if (!isConnected) {
        // console.log(`Simulated command: ${command}`);
        // return; 
    }
    
    console.log(`Sending command to ESP32: ${command}`);
    fetch(`/command?cmd=${command}`);
}

// Fetch data from ESP32 (real implementation)
function fetchDataFromESP32() {
    fetch(`/sensors`)
        .then(response => response.json())
        .then(data => {
            // Update UI with real data
            if(data.ultrasonic1) ultrasonic1El.innerHTML = data.ultrasonic1 + '<span class="sensor-unit">cm</span>';
            if(data.ultrasonic2) ultrasonic2El.innerHTML = data.ultrasonic2 + '<span class="sensor-unit">cm</span>';
            if(data.smoke) smokeValueEl.innerHTML = data.smoke + '<span class="sensor-unit">ppm</span>';
            if(data.battery) batteryLevelEl.textContent = data.battery + '%';
        })
        .catch(error => {
            console.error('Error fetching data from ESP32:', error);
            // showMessage('Error fetching data from ESP32', 'error');
        });
}

// Show message to user
function showMessage(message, type = 'info') {
    // Create message element
    const messageEl = document.createElement('div');
    messageEl.className = `message ${type}`;
    messageEl.textContent = message;
    messageEl.style.cssText = `
        position: fixed;
        top: 20px;
        right: 20px;
        padding: 15px 25px;
        border-radius: 8px;
        color: white;
        font-weight: bold;
        z-index: 1000;
        animation: slideIn 0.3s ease-out;
        max-width: 400px;
    `;
    
    // Set background color based on message type
    if (type === 'success') {
        messageEl.style.background = 'linear-gradient(90deg, #4CAF50, #45a049)';
    } else if (type === 'error') {
        messageEl.style.background = 'linear-gradient(90deg, #f44336, #d32f2f)';
    } else if (type === 'warning') {
        messageEl.style.background = 'linear-gradient(90deg, #ff9800, #f57c00)';
    } else {
        messageEl.style.background = 'linear-gradient(90deg, #2196F3, #1976D2)';
    }
    
    // Add to DOM
    document.body.appendChild(messageEl);
    
    // Remove after 3 seconds
    setTimeout(() => {
        messageEl.style.animation = 'slideOut 0.3s ease-in';
        setTimeout(() => {
            if (messageEl.parentNode) {
                messageEl.parentNode.removeChild(messageEl);
            }
        }, 300);
    }, 3000);
    
    // Add CSS for animations if not already present
    if (!document.getElementById('message-animations')) {
        const style = document.createElement('style');
        style.id = 'message-animations';
        style.textContent = `
            @keyframes slideIn {
                from { transform: translateX(100%); opacity: 0; }
                to { transform: translateX(0); opacity: 1; }
            }
            @keyframes slideOut {
                from { transform: translateX(0); opacity: 1; }
                to { transform: translateX(100%); opacity: 0; }
            }
        `;
        document.head.appendChild(style);
    }
}

// Update connection status UI
function updateConnectionStatus(connected) {
    isConnected = connected;
    
    if (connected) {
        esp32StatusIndicator.className = 'status-indicator online';
        connectionStatusEl.textContent = 'Connected';
        connectionStatusEl.style.color = '#00ff00';
    } else {
        esp32StatusIndicator.className = 'status-indicator offline';
        connectionStatusEl.textContent = 'Disconnected';
        connectionStatusEl.style.color = '#ff0000';
    }
}

// Initialize application when DOM is loaded
document.addEventListener('DOMContentLoaded', init);
    </script>
</body>
</html>
)rawliteral";


// ==========================================
//              LOGIC
// ==========================================

long getDistance() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  long duration = pulseIn(ECHO, HIGH, 30000); // 30ms timeout
  if (duration == 0) return 999; // No echo
  return duration * 0.034 / 2;
}

void setup() {
  Serial.begin(115200);

  // Pin Setup
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  s1.attach(SERVO1);
  s2.attach(SERVO2);
  s3.attach(SERVO3);
  s4.attach(SERVO4);
  
  // Neutral
  s1.write(90); s2.write(90); s3.write(90); s4.write(90);

  // WiFi
  WiFi.softAP(ssid, password);
  Serial.println("AP Started: " + WiFi.softAPIP().toString());

  // Server Routes
  server.on("/", []() { server.send(200, "text/html", html_content); });
  
  server.on("/command", []() {
    String cmd = server.arg("cmd");
    if (cmd == "START_ROBOT") { moveForward = true; moveBackward = false; }
    else if (cmd == "BACK_ROBOT") { moveForward = false; moveBackward = true; }
    else if (cmd == "STOP_ROBOT") { moveForward = false; moveBackward = false; }
    server.send(200, "text/plain", "OK");
  });

  server.on("/sensors", []() {
    String json = "{ \"ultrasonic1\": " + String(distanceFront) + " }";
    server.send(200, "application/json", json);
  });

  server.onNotFound([]() {
     // Handle /servo/ID?position=VAL manually to avoid many routes
     if (server.uri().startsWith("/servo/")) {
        int id = server.uri().substring(7).toInt();
        int pos = server.arg("position").toInt();
        if (id == 13) s1.write(pos);
        if (id == 12) s2.write(pos);
        if (id == 14) s3.write(pos);
        if (id == 27) s4.write(pos);
        server.send(200, "text/plain", "OK");
     } else {
        server.send(404, "text/plain", "Not Found");
     }
  });

  server.begin();
}

void loop() {
  server.handleClient();
  unsigned long currentMillis = millis();

  // --- NON-BLOCKING SENSOR ---
  if (currentMillis - lastSensorTime >= 200) {
    long d = getDistance();
    if (d > 0 && d < 400) distanceFront = d;
    lastSensorTime = currentMillis;
  }

  // --- NON-BLOCKING MOVEMENT ---
  if (moveForward || moveBackward) {
    if (currentMillis - lastMoveTime >= 300) { // 300ms delay from original code
      lastMoveTime = currentMillis;
      
      if (moveForward) {
          // Check collision
          if (distanceFront < 20 && distanceFront > 0) {
              moveForward = false; // Stop if too close
          } else {
              // Forward Gait
              if (moveStep == 0) {
                  s1.write(60); s3.write(60); s2.write(120); s4.write(120);
                  moveStep = 1;
              } else {
                  s1.write(120); s3.write(120); s2.write(60); s4.write(60);
                  moveStep = 0;
              }
          }
      } else if (moveBackward) {
           // Backward Gait (Reverse of Forward)
          if (moveStep == 0) {
              s1.write(120); s3.write(120); s2.write(60); s4.write(60);
              moveStep = 1;
          } else {
              s1.write(60); s3.write(60); s2.write(120); s4.write(120);
              moveStep = 0;
          }
      }
    }
  }
}
