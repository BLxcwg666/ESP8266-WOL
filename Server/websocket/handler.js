const { v4: uuidv4 } = require('uuid');

const createWebSocketHandler = (wss, devices, authToken) => {
  wss.on('connection', (ws, req) => {
    const deviceId = uuidv4();
    const clientIp = req.socket.remoteAddress;
    
    console.log(`New connection: ${deviceId} - ${clientIp}`);
    
    const device = {
      id: deviceId,
      ws: ws,
      ip: clientIp,
      connectedAt: new Date(),
      authenticated: false,
      status: {}
    };
    
    devices.set(deviceId, device);
    
    ws.send(JSON.stringify({
      type: 'welcome',
      deviceId: deviceId,
      message: 'authorization needed'
    }));
    
    ws.on('message', (message) => {
      try {
        const data = JSON.parse(message);
        console.log(`Received msg from ${deviceId}:`, data);
        
        switch (data.type) {
          case 'auth':
            if (data.token === authToken) {
              device.authenticated = true;
              ws.send(JSON.stringify({
                type: 'auth_success',
                message: 'auth successfully',
              }));
              console.log(`Device ${deviceId} auth successfully`);
            } else {
              ws.send(JSON.stringify({
                type: 'auth_failed',
                message: 'auth failed：token error'
              }));
              console.log(`Device ${deviceId} auth failed`);
              setTimeout(() => ws.close(), 1000);
            }
            break;
            
          case 'ping':
            ws.send(JSON.stringify({
              type: 'pong',
              timestamp: new Date().toISOString()
            }));
            break;
            
          case 'status':
            device.status = data.status;
            console.log(`Updated device ${device.id}:`, data.status);
            break;
            
          case 'wol_result':
            console.log(`WOL result from ${device.id}:`, {
              mac: data.mac,
              success: data.success,
              timestamp: data.timestamp
            });
            break;
            
          case 'data':
            console.log(`Device data from ${device.id}:`, data.payload);
            break;
            
          default:
            console.log(`unknown msg type: ${data.type}`);
        }
      } catch (error) {
        console.error('Failed to process msg:', error);
      }
    });
    
    ws.on('close', () => {
      console.log(`Device ${deviceId} disconnected`);
      devices.delete(deviceId);
    });
    
    ws.on('error', (error) => {
      console.error(`Device ${deviceId} websocket error:`, error);
    });
  });
};

module.exports = createWebSocketHandler;