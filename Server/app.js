const express = require('express');
const http = require('http');
const WebSocket = require('ws');
const fs = require('fs').promises;
const path = require('path');

const config = require('./config');

const auth = require('./utils/auth');
const createAuthRoutes = require('./routes/auth');
const createDeviceRoutes = require('./routes/devices');
const createWolRoutes = require('./routes/wol');
const createWebSocketHandler = require('./websocket/handler');

const app = express();
const server = http.createServer(app);

const wss = new WebSocket.Server({server, path: config.wsPath});

const devices = new Map();
const savedDevices = new Map();
const DEVICES_FILE = path.join(__dirname, config.devicesFile);

async function loadSavedDevices() {
  try {
    const data = await fs.readFile(DEVICES_FILE, 'utf8');
    const devicesArray = JSON.parse(data);
    savedDevices.clear();
    devicesArray.forEach(device => {
      savedDevices.set(device.id, device);
    });
    console.log(`Loaded ${savedDevices.size} saved devices`);
  } catch (error) {
    if (error.code === 'ENOENT') {
      await saveSavedDevices();
    } else {
      console.error('Failed to load devices file:', error);
    }
  }
}

async function saveSavedDevices() {
  try {
    const devicesArray = Array.from(savedDevices.values());
    await fs.writeFile(DEVICES_FILE, JSON.stringify(devicesArray, null, 2));
    console.log('saved devices');
  } catch (error) {
    console.error('Failed to save devices file:', error);
  }
}

app.use(express.json());
app.use(express.static('public'));

app.use('/api', createAuthRoutes(config.adminPassword, config.jwtSecret));
app.use('/api/devices', auth(config.jwtSecret), createDeviceRoutes(devices));
app.use('/api/wol', auth(config.jwtSecret), createWolRoutes(devices, savedDevices, saveSavedDevices));

createWebSocketHandler(wss, devices, config.authToken);

app.get('/api/health', (req, res) => {
  res.json({
    status: 'ok',
    connectedDevices: devices.size,
    savedDevices: savedDevices.size,
    uptime: process.uptime()
  });
});

server.listen(config.port, async () => {
  await loadSavedDevices();
  console.log(`Running at :${config.port}`);
});