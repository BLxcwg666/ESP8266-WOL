require('dotenv').config();

const config = {
  port: process.env.PORT || 3000,
  authToken: process.env.AUTH_TOKEN || 'bbbbbbbbbbb',
  adminPassword: process.env.ADMIN_PASSWORD || 'bbbbbbbbbbbbbbbbbbb',
  jwtSecret: process.env.JWT_SECRET || 'your-jwt-secret-key-change-in-production',
  wsPath: '/ws',
  devicesFile: 'wol-devices.json',
  isDevelopment: process.env.NODE_ENV === 'development'
};

module.exports = config;