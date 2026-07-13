// Layered config for api-gateway (SOFTWARE_ARCHITECTURE.md §6, env-var layer —
// the Node service reads the same 12-factor escape hatch the C++ services'
// ConfigStore does; layers 1-3 (compiled defaults/platform.yaml/per-service
// file) apply equally here via defaults below plus deployment-profile
// overrides, not re-implemented as a separate YAML parser in two languages).

function required(name, fallback) {
  const value = process.env[name] ?? fallback;
  if (value === undefined) {
    throw new Error(`missing required config: ${name}`);
  }
  return value;
}

export const config = {
  httpPort: Number(process.env.UAOP_GATEWAY_HTTP_PORT ?? 8080),
  wsPort: Number(process.env.UAOP_GATEWAY_WS_PORT ?? 8081),
  natsUrl: process.env.UAOP_NATS_URL ?? 'nats://localhost:4222',
  redisUrl: process.env.UAOP_REDIS_URL ?? 'redis://localhost:6379',
  grpcTarget: process.env.UAOP_GATEWAY_GRPC_TARGET ?? 'localhost:50051',
  // Dev-only signing secret (SECURITY.md §4: local identity provider Phase
  // 1-3, real HSM/KMS-backed signing is a Phase 4 hardening item — this is
  // explicitly NOT a production secret path).
  jwtSecret: required('UAOP_GATEWAY_JWT_SECRET', 'dev-only-insecure-secret-change-me'),
  jwtTtlSeconds: Number(process.env.UAOP_GATEWAY_JWT_TTL_SECONDS ?? 60 * 60), // <=60min, SECURITY.md §3/R1-F5
  heartbeatMs: Number(process.env.UAOP_GATEWAY_WS_HEARTBEAT_MS ?? 5000), // API_SPECIFICATION.md §5
};
