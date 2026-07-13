// Dynamic gRPC client factory for the gateway/v1 services (API_SPECIFICATION.md
// §6). Loads the real .proto files at runtime via @grpc/proto-loader instead
// of requiring generated stubs — sidesteps needing a C++-style protobuf
// codegen step in this Node service (ADR-0004: "gRPC-Gateway tooling is
// mature" — proto-loader's dynamic loading is exactly that maturity payoff).

import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { createRequire } from 'node:module';

import * as grpc from '@grpc/grpc-js';
import protoLoader from '@grpc/proto-loader';

const require = createRequire(import.meta.url);
const __dirname = path.dirname(fileURLToPath(import.meta.url));

// backend/api-gateway/src/grpc -> repo root -> api/proto
const PROTO_ROOT = path.resolve(__dirname, '../../../../api/proto');
// google/protobuf/*.proto well-known types are not vendored in api/proto
// (they ship with protoc); google-proto-files carries copies for exactly
// this dynamic-loading use case.
const WELL_KNOWN_ROOT = path.dirname(require.resolve('google-proto-files/package.json'));

function loadPackage(protoRelativePath, packageName) {
  const packageDefinition = protoLoader.loadSync(protoRelativePath, {
    keepCase: false,
    longs: String,
    enums: String,
    defaults: true,
    oneofs: true,
    includeDirs: [PROTO_ROOT, WELL_KNOWN_ROOT],
  });
  const descriptor = grpc.loadPackageDefinition(packageDefinition);
  return packageName.split('.').reduce((obj, key) => obj[key], descriptor);
}

const GATEWAY_SERVICES = {
  VehicleCommandService: 'uaop/gateway/v1/command.proto',
  MissionService: 'uaop/gateway/v1/mission.proto',
  ParameterService: 'uaop/gateway/v1/parameter.proto',
  TelemetryQueryService: 'uaop/gateway/v1/telemetry_query.proto',
  LogService: 'uaop/gateway/v1/log.proto',
  ComplianceService: 'uaop/gateway/v1/compliance.proto',
};

const clientCache = new Map();

// Returns a connected gRPC client for one of the six gateway/v1 services,
// memoized per (serviceName, target) pair. `target` defaults to
// config.grpcTarget — no real backend listens there yet in Phase 1 (the
// engines land M2+); the client connects lazily on first RPC, so
// constructing it here never blocks gateway startup on a service that
// doesn't exist yet.
export function getServiceClient(serviceName, target, credentials = grpc.credentials.createInsecure()) {
  const protoPath = GATEWAY_SERVICES[serviceName];
  if (!protoPath) {
    throw new Error(`unknown gateway service: ${serviceName}`);
  }
  const cacheKey = `${serviceName}@${target}`;
  if (clientCache.has(cacheKey)) {
    return clientCache.get(cacheKey);
  }
  const ServiceCtor = loadPackage(protoPath, `uaop.gateway.v1.${serviceName}`);
  const client = new ServiceCtor(target, credentials);
  clientCache.set(cacheKey, client);
  return client;
}

export function clearClientCache() {
  clientCache.clear();
}
