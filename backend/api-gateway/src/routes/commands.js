// VehicleCommandService forwarding (API_SPECIFICATION.md §4): commands are
// asynchronous with mandatory resolution — this route only ever returns 202 +
// correlation_id, never a synchronous ACCEPTED/REJECTED (that requires a real
// vehicle-manager on the other end of the gRPC call, which doesn't exist
// until M3+; this route's job is the translation layer, proven against a
// mock server in tests since the real one isn't built yet).

import { Router } from 'express';

import { getServiceClient } from '../grpc/client.js';
import { config } from '../config.js';

const REST_TO_COMMAND_KIND = {
  ARM: 'COMMAND_KIND_ARM',
  DISARM: 'COMMAND_KIND_DISARM',
  MODE: 'COMMAND_KIND_MODE',
  RTL: 'COMMAND_KIND_RTL',
  TAKEOFF: 'COMMAND_KIND_TAKEOFF',
  LAND: 'COMMAND_KIND_LAND',
  MISSION_START: 'COMMAND_KIND_MISSION_START',
};

export function commandsRouter({ grpcTarget = config.grpcTarget } = {}) {
  const router = Router();

  router.post('/vehicles/:id/commands', (req, res) => {
    const { command, params } = req.body ?? {};
    const commandKind = REST_TO_COMMAND_KIND[command];
    if (!commandKind) {
      return res.status(400).json({
        error: { code: 'ERROR_CODE_INVALID_ARGUMENT', message: `unknown command: ${command}` },
      });
    }

    const client = getServiceClient('VehicleCommandService', grpcTarget);
    const request = {
      vehicleId: req.params.id,
      command: commandKind,
      params: params ?? {},
      idempotencyKey: req.headers['idempotency-key'] ?? '',
    };

    client.submitCommand(request, (err, response) => {
      if (err) {
        return res.status(502).json({
          error: { code: 'ERROR_CODE_UNAVAILABLE', message: `command forwarding failed: ${err.message}` },
        });
      }
      res.status(202).json({ correlation_id: response.correlationId });
    });
  });

  router.get('/commands/:correlationId', (req, res) => {
    const client = getServiceClient('VehicleCommandService', grpcTarget);
    client.getCommandResult({ correlationId: req.params.correlationId }, (err, response) => {
      if (err) {
        return res.status(502).json({
          error: { code: 'ERROR_CODE_UNAVAILABLE', message: `command result fetch failed: ${err.message}` },
        });
      }
      res.json(response.result);
    });
  });

  return router;
}
