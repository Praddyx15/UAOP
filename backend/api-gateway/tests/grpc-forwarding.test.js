// Proves the REST->gRPC forwarding layer actually works end-to-end, using a
// real gRPC server built from the same command.proto the gateway loads —
// no mocked gRPC client, no assumed wire shape. The real vehicle-manager
// doesn't exist until M3+; this test is what stands in for it until then.

import { test, describe, before, after } from 'node:test';
import assert from 'node:assert/strict';
import express from 'express';
import http from 'node:http';

import * as grpc from '@grpc/grpc-js';

import { getServiceClient, clearClientCache } from '../src/grpc/client.js';
import { commandsRouter } from '../src/routes/commands.js';

let mockServer;
let mockServerPort;
let httpServer;
let httpPort;

before(async () => {
  // A minimal real gRPC server implementing VehicleCommandService, loaded
  // from the identical proto the client side uses.
  mockServer = new grpc.Server();
  const ServiceCtor = getServiceClient; // reuse the loader indirectly below
  void ServiceCtor;

  const protoLoader = await import('@grpc/proto-loader');
  const path = await import('node:path');
  const { fileURLToPath } = await import('node:url');
  const { createRequire } = await import('node:module');
  const require = createRequire(import.meta.url);
  const __dirname = path.dirname(fileURLToPath(import.meta.url));
  // tests/ is one level shallower than src/grpc/ (client.js), so this needs
  // 3 levels up to repo root, not 4 — a real bug caught by actually running
  // this, not a copy-paste that happened to be right.
  const PROTO_ROOT = path.resolve(__dirname, '../../../api/proto');
  const WELL_KNOWN_ROOT = path.dirname(require.resolve('google-proto-files/package.json'));

  const packageDefinition = protoLoader.loadSync('uaop/gateway/v1/command.proto', {
    keepCase: false, longs: String, enums: String, defaults: true, oneofs: true,
    includeDirs: [PROTO_ROOT, WELL_KNOWN_ROOT],
  });
  const descriptor = grpc.loadPackageDefinition(packageDefinition);
  const VehicleCommandService = descriptor.uaop.gateway.v1.VehicleCommandService;

  mockServer.addService(VehicleCommandService.service, {
    submitCommand: (call, callback) => {
      callback(null, { correlationId: `corr-${call.request.vehicleId}-${call.request.command}` });
    },
    getCommandResult: (call, callback) => {
      callback(null, {
        result: {
          correlationId: call.request.correlationId,
          vehicleId: 'veh-1',
          command: 'COMMAND_KIND_ARM',
          status: 'COMMAND_STATUS_ACCEPTED',
        },
      });
    },
  });

  mockServerPort = await new Promise((resolve, reject) => {
    mockServer.bindAsync('127.0.0.1:0', grpc.ServerCredentials.createInsecure(), (err, port) => {
      if (err) reject(err); else resolve(port);
    });
  });

  const app = express();
  app.use(express.json());
  app.use(commandsRouter({ grpcTarget: `127.0.0.1:${mockServerPort}` }));
  httpServer = http.createServer(app);
  await new Promise((resolve) => httpServer.listen(0, resolve));
  httpPort = httpServer.address().port;
});

after(async () => {
  clearClientCache();
  await new Promise((resolve) => httpServer.close(resolve));
  mockServer.forceShutdown();
});

describe('commands route -> gRPC forwarding (real gRPC server)', () => {
  test('POST /vehicles/:id/commands forwards to SubmitCommand and returns 202 + correlation_id', async () => {
    const res = await fetch(`http://localhost:${httpPort}/vehicles/veh-42/commands`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ command: 'ARM', params: {} }),
    });
    assert.equal(res.status, 202);
    const body = await res.json();
    assert.equal(body.correlation_id, 'corr-veh-42-COMMAND_KIND_ARM');
  });

  test('POST with an unknown command is rejected before hitting gRPC', async () => {
    const res = await fetch(`http://localhost:${httpPort}/vehicles/veh-42/commands`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ command: 'NOT_A_COMMAND' }),
    });
    assert.equal(res.status, 400);
  });

  test('GET /commands/:correlationId forwards to GetCommandResult', async () => {
    const res = await fetch(`http://localhost:${httpPort}/commands/corr-abc`);
    assert.equal(res.status, 200);
    const body = await res.json();
    assert.equal(body.correlationId, 'corr-abc');
    assert.equal(body.status, 'COMMAND_STATUS_ACCEPTED');
  });
});
