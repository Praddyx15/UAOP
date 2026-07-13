// Express auth middleware: verifies the bearer JWT, checks the denylist,
// attaches req.user. Route-level RBAC (SECURITY.md §3: "enforced twice —
// gateway route policy + service-level checks") is requireRole() below; the
// service-level re-check is each backend service's own job, not this
// gateway's — defense in depth means the gateway is not the only wall.

import { verifyToken } from './jwt.js';

export function authenticate(denylist) {
  return async (req, res, next) => {
    const header = req.headers.authorization ?? '';
    const [scheme, token] = header.split(' ');
    if (scheme !== 'Bearer' || !token) {
      return res.status(401).json({ error: { code: 'ERROR_CODE_UNAUTHENTICATED', message: 'missing bearer token' } });
    }

    let claims;
    try {
      claims = verifyToken(token);
    } catch {
      return res.status(401).json({ error: { code: 'ERROR_CODE_UNAUTHENTICATED', message: 'invalid or expired token' } });
    }

    if (await denylist.isRevoked(claims.jti)) {
      return res.status(401).json({ error: { code: 'ERROR_CODE_UNAUTHENTICATED', message: 'token revoked' } });
    }

    req.user = { subject: claims.sub, role: claims.role, org: claims.org, jti: claims.jti, exp: claims.exp };
    next();
  };
}

export function requireRole(...allowedRoles) {
  return (req, res, next) => {
    if (!req.user || !allowedRoles.includes(req.user.role)) {
      return res.status(403).json({ error: { code: 'ERROR_CODE_PERMISSION_DENIED', message: 'role not permitted for this route' } });
    }
    next();
  };
}
