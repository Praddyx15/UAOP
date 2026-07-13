import { Router } from 'express';

export function healthRouter() {
  const router = Router();
  router.get('/healthz', (req, res) => {
    res.json({ status: 'OK' });
  });
  return router;
}
