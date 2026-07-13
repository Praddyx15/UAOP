// Redis-backed token denylist (SECURITY.md §3): immediate revocation on
// logout/compromise/role-change, interim until OIDC session management
// arrives in Phase 4. Keyed on the token's `jti`, TTL matches the token's
// own remaining lifetime — DATABASE.md §6's "Redis: nothing durable lives
// here" rule applies: a Redis loss just means revoked tokens work again
// until they'd have expired anyway, not a silent security hole (the token
// itself still expires on schedule).

const KEY_PREFIX = 'uaop:gw:denylist:';

export class Denylist {
  constructor(redisClient) {
    this.redis = redisClient;
  }

  async revoke(jti, ttlSeconds) {
    if (ttlSeconds <= 0) {
      return; // already expired naturally, nothing to deny
    }
    await this.redis.set(KEY_PREFIX + jti, '1', 'EX', ttlSeconds);
  }

  async isRevoked(jti) {
    const value = await this.redis.get(KEY_PREFIX + jti);
    return value !== null;
  }
}
