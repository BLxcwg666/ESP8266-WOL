const express = require('express');
const router = express.Router();
const WebSocket = require('ws');
const { v4: uuidv4 } = require('uuid');

const createWolRoutes = (devices, savedDevices, saveSavedDevices) => {
  router.post('/send', (req, res) => {
    const { mac, deviceId } = req.body;
    
    if (!mac || !/^([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})$/.test(mac)) {
      return res.status(400).json({ error: 'invalid mac address' });
    }
    
    if (deviceId) {
      const device = devices.get(deviceId);
      if (!device || !device.authenticated) {
        return res.status(404).json({ error: 'device not found' });
      }
      
      device.ws.send(JSON.stringify({type: 'wol', mac: mac}));
      
      res.json({ success: true, message: 'sended wol command' });
    } else {
      let sent = 0;
      devices.forEach(device => {
        if (device.authenticated && device.ws.readyState === WebSocket.OPEN) {
          device.ws.send(JSON.stringify({
            type: 'wol',
            mac: mac
          }));
          sent++;
        }
      });
      
      res.json({ success: true, message: `sended wol command to ${sent} device(s)` });
    }
  });

  router.get('/devices', (req, res) => {
    const deviceList = Array.from(savedDevices.values());
    res.json(deviceList);
  });

  router.post('/devices', async (req, res) => {
    const { name, mac, description } = req.body;
    
    if (!name || !mac) {
      return res.status(400).json({ error: 'name and mac address are required' });
    }
    
    if (!/^([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})$/.test(mac)) {
      return res.status(400).json({ error: 'invalid mac address' });
    }
    
    const id = uuidv4();
    const device = {
      id,
      name,
      mac: mac.toUpperCase().replace(/:/g, '-'),
      description: description || '',
      createdAt: new Date()
    };
    
    savedDevices.set(id, device);
    await saveSavedDevices();
    res.json({ success: true, device });
  });

  router.delete('/devices/:id', async (req, res) => {
    const { id } = req.params;
    
    if (savedDevices.has(id)) {
      savedDevices.delete(id);
      await saveSavedDevices();
      res.json({ success: true });
    } else {
      res.status(404).json({ error: 'device not found' });
    }
  });

  router.put('/devices/:id', async (req, res) => {
    const { id } = req.params;
    const { name, mac, description } = req.body;
    
    const device = savedDevices.get(id);
    if (!device) {
      return res.status(404).json({ error: 'device not found' });
    }
    
    if (name) device.name = name;
    if (mac) {
      if (!/^([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})$/.test(mac)) {
        return res.status(400).json({ error: 'invalid mac address' });
      }
      device.mac = mac.toUpperCase().replace(/:/g, '-');
    }
    if (description !== undefined) device.description = description;
    
    await saveSavedDevices();
    res.json({ success: true, device });
  });

  return router;
};

module.exports = createWolRoutes;