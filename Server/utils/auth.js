const auth = (adminPassword) => {
  return (req, res, next) => {
    const token = req.headers.authorization?.replace('Bearer ', '');
    if (!token) {
      return res.status(401).json({ error: 'unauthorized' });
    }
    
    if (token !== 'admin-token-' + adminPassword) {
      return res.status(401).json({ error: 'invalid token' });
    }
    
    next();
  };
};

module.exports = auth;