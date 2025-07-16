const jwt = require('jsonwebtoken');

const auth = (jwtSecret) => {
  return (req, res, next) => {
    const token = req.headers.authorization?.replace('Bearer ', '');
    if (!token) {
      return res.status(401).json({ error: 'unauthorized' });
    }
    
    try {
      const decoded = jwt.verify(token, jwtSecret);
      req.user = decoded;
      next();
    } catch (error) {
      if (error.name === 'TokenExpiredError') {
        return res.status(401).json({ error: 'token expired' });
      }
      return res.status(401).json({ error: 'invalid token' });
    }
  };
};

module.exports = auth;