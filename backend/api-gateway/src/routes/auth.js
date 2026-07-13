// Dev-mode login (SECURITY.md §3 names Argon2id-hashed credentials against a
// local identity provider — that needs the `auth.users` Postgres schema,
// M1.7, which doesn't exist yet). This route issues a real, correctly-scoped
// JWT but skips real credential verification, gated behind
// UAOP_GATEWAY_DEV_AUTH=1 so it can never be accidentally live in a profile
// that isn't workstation. Wiring this to real Argon2id verification against
// `auth.users` is M1.7+ follow-up work, not silently pretended-done here.

import { Router } from 'express';

import { issueToken } from '../auth/jwt.js';
import { isValidRole } from '../auth/roles.js';

export function authRouter({ denylist }) {
  const router = Router();

  router.post('/auth/dev-login', (req, res) => {
    if (process.env.UAOP_GATEWAY_DEV_AUTH !== '1') {
      return res.status(404).json({ error: { code: 'ERROR_CODE_NOT_FOUND', message: 'not found' } });
    }
    const { subject, role, org } = req.body ?? {};
    if (!subject || !isValidRole(role)) {
      return res.status(400).json({ error: { code: 'ERROR_CODE_INVALID_ARGUMENT', message: 'subject and a valid role are required' } });
    }
    const { token } = issueToken({ subject, role, org });
    res.json({ token });
  });

  router.post('/auth/logout', async (req, res) => {
    if (!req.user) {
      return res.status(401).json({ error: { code: 'ERROR_CODE_UNAUTHENTICATED', message: 'not authenticated' } });
    }
    const remainingTtl = req.user.exp - Math.floor(Date.now() / 1000);
    await denylist.revoke(req.user.jti, remainingTtl);
    res.status(204).end();
  });

  return router;
}
