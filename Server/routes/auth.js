const express = require('express');
const jwt = require('jsonwebtoken');
const router = express.Router();

const createAuthRoutes = (adminPassword, jwtSecret) => {
  router.post('/login', (req, res) => {
    const { password } = req.body;
    
    if (password === adminPassword) {
      const payload = {
        role: 'admin',
        timestamp: Date.now()
      };
      
      const token = jwt.sign(payload, jwtSecret, {
        expiresIn: '7d'
      });
      
      res.json({ 
        success: true, 
        token,
        expiresIn: 7 * 24 * 60 * 60 * 1000
      });
    } else {
      res.status(401).json({ success: false, message: 'invalid password' });
    }
  });

  return router;
};

module.exports = createAuthRoutes;