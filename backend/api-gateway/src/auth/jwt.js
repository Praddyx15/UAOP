// JWT issuing/verification (SECURITY.md §3: access tokens <=60min with
// refresh rotation). This module only signs/verifies — revocation is
// denylist.js's job, kept separate so a compromised-token response doesn't
// require touching signing logic.

import jwt from 'jsonwebtoken';
import { randomUUID } from 'node:crypto';

import { config } from '../config.js';
import { isValidRole } from './roles.js';

export function issueToken({ subject, role, org }) {
  if (!isValidRole(role)) {
    throw new Error(`unknown role: ${role}`);
  }
  const jti = randomUUID();
  const token = jwt.sign(
    { sub: subject, role, org: org ?? null, jti },
    config.jwtSecret,
    { expiresIn: config.jwtTtlSeconds, algorithm: 'HS256' },
  );
  return { token, jti };
}

// Returns the decoded claims, or throws (callers convert to a 401 — this
// module has no HTTP-layer knowledge, matching the boundary discipline the
// C++ services/common Error taxonomy uses on the other side of the gateway).
export function verifyToken(token) {
  return jwt.verify(token, config.jwtSecret, { algorithms: ['HS256'] });
}
