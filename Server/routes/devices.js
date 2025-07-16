const express = require('express');
const router = express.Router();

const createDeviceRoutes = (devices) => {
  router.get('/', (req, res) => {
    const deviceList = Array.from(devices.values()).map(device => ({
      id: device.id,
      ip: device.ip,
      connectedAt: device.connectedAt,
      authenticated: device.authenticated,
      status: device.status
    }));
    res.json(deviceList);
  });

  router.post('/:id/disconnect', (req, res) => {
    const { id } = req.params;
    const device = devices.get(id);
    
    if (!device) {
      return res.status(404).json({ error: 'device not found' });
    }
    
    device.ws.close();
    res.json({ success: true });
  });

  return router;
};

module.exports = createDeviceRoutes;