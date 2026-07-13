import { test, describe } from 'node:test';
import assert from 'node:assert/strict';

import Redis from 'ioredis';

import { issueToken, verifyToken } from '../src/auth/jwt.js';
import { Denylist } from '../src/auth/denylist.js';
import { ROLES } from '../src/auth/roles.js';

describe('jwt', () => {
  test('issues a token carrying subject/role/jti and verifies it back', () => {
    const { token, jti } = issueToken({ subject: 'user-1', role: ROLES.PILOT, org: 'org-1' });
    const claims = verifyToken(token);
    assert.equal(claims.sub, 'user-1');
    assert.equal(claims.role, ROLES.PILOT);
    assert.equal(claims.org, 'org-1');
    assert.equal(claims.jti, jti);
    assert.ok(claims.exp > Math.floor(Date.now() / 1000));
  });

  test('rejects an unknown role', () => {
    assert.throws(() => issueToken({ subject: 'user-1', role: 'NOT_A_ROLE' }));
  });

  test('verifyToken throws on a tampered token', () => {
    const { token } = issueToken({ subject: 'user-1', role: ROLES.OBSERVER });
    assert.throws(() => verifyToken(token.slice(0, -2) + 'xx'));
  });
});

describe('denylist (real Redis)', () => {
  test('a fresh jti is not revoked, then is revoked after revoke()', async () => {
    const redis = new Redis(process.env.UAOP_REDIS_URL ?? 'redis://localhost:6379');
    const denylist = new Denylist(redis);
    const { jti } = issueToken({ subject: 'user-2', role: ROLES.MAINTENANCE });

    assert.equal(await denylist.isRevoked(jti), false);
    await denylist.revoke(jti, 30);
    assert.equal(await denylist.isRevoked(jti), true);

    redis.disconnect();
  });

  test('revoke() with a non-positive TTL is a no-op (already-expired token)', async () => {
    const redis = new Redis(process.env.UAOP_REDIS_URL ?? 'redis://localhost:6379');
    const denylist = new Denylist(redis);
    await denylist.revoke('some-jti-already-expired', 0);
    assert.equal(await denylist.isRevoked('some-jti-already-expired'), false);
    redis.disconnect();
  });
});
