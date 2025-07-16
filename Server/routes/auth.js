const express = require('express');
const router = express.Router();

const createAuthRoutes = (adminPassword) => {
  router.post('/login', (req, res) => {
    const { password } = req.body;
    
    if (password === adminPassword) {
      const token = 'admin-token-' + adminPassword;
      res.json({ 
        success: true, 
        token,
        expiresIn: 365 * 24 * 60 * 60 * 1000
      });
    } else {
      res.status(401).json({ success: false, message: 'invalid password' });
    }
  });

  return router;
};

module.exports = createAuthRoutes;