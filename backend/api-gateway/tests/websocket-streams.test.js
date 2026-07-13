// Real end-to-end test against the live NATS server (M1.5's provisioned
// TELEMETRY stream) — publishes a raw message on a telemetry subject and
// verifies the WebSocket subscription protocol relays it as a correctly
// shaped frame, including gap accounting under rate shaping.

import { test, describe } from 'node:test';
import assert from 'node:assert/strict';
import http from 'node:http';

import { WebSocket } from 'ws';
import { connect as natsConnect, StringCodec } from 'nats';

import { createStreamServer } from '../src/streams/websocket.js';

const NATS_URL = process.env.UAOP_NATS_URL ?? 'nats://localhost:4222';
const sc = StringCodec();

async function withServer(fn) {
  const nats = await natsConnect({ servers: NATS_URL });
  const server = http.createServer();
  createStreamServer({ server, natsConnection: nats, heartbeatMs: 60_000 }); // heartbeat out of the way for these tests
  await new Promise((resolve) => server.listen(0, resolve));
  const port = server.address().port;
  try {
    await fn({ nats, port });
  } finally {
    await new Promise((resolve) => server.close(resolve));
    await nats.close();
  }
}

function waitForFrame(ws, predicate, timeoutMs = 3000) {
  return new Promise((resolve, reject) => {
    const timer = setTimeout(() => reject(new Error('timed out waiting for frame')), timeoutMs);
    ws.on('message', (raw) => {
      const frame = JSON.parse(raw.toString());
      if (predicate(frame)) {
        clearTimeout(timer);
        resolve(frame);
      }
    });
  });
}

describe('websocket streams (real NATS)', () => {
  test('relays a telemetry message on the subscribed subject as a seq/ts/payload frame', async () => {
    await withServer(async ({ nats, port }) => {
      const ws = new WebSocket(`ws://localhost:${port}/ws/v1`);
      await new Promise((resolve) => ws.on('open', resolve));

      ws.send(JSON.stringify({
        op: 'subscribe',
        streams: [{ type: 'telemetry', vehicle: 'veh-test-1', categories: ['position'] }],
      }));
      // give the subscribe handshake a moment to establish the NATS subscription
      await new Promise((resolve) => setTimeout(resolve, 200));

      nats.publish('uaop.telemetry.v1.veh-test-1.position', sc.encode('fake-position-payload'));

      const frame = await waitForFrame(ws, (f) => f.stream === 'telemetry.veh-test-1.position');
      assert.equal(frame.seq, 1);
      assert.ok(frame.ts);
      assert.equal(Buffer.from(frame.payload, 'base64').toString(), 'fake-position-payload');

      ws.close();
    });
  });

  test('does not relay messages on a subject the client did not subscribe to', async () => {
    await withServer(async ({ nats, port }) => {
      const ws = new WebSocket(`ws://localhost:${port}/ws/v1`);
      await new Promise((resolve) => ws.on('open', resolve));

      ws.send(JSON.stringify({
        op: 'subscribe',
        streams: [{ type: 'telemetry', vehicle: 'veh-test-2', categories: ['position'] }],
      }));
      await new Promise((resolve) => setTimeout(resolve, 200));

      let receivedUnexpected = false;
      ws.on('message', (raw) => {
        const frame = JSON.parse(raw.toString());
        if (frame.stream === 'telemetry.veh-test-2.battery') {
          receivedUnexpected = true;
        }
      });

      nats.publish('uaop.telemetry.v1.veh-test-2.battery', sc.encode('should-not-arrive'));
      await new Promise((resolve) => setTimeout(resolve, 300));

      assert.equal(receivedUnexpected, false);
      ws.close();
    });
  });

  test('rate_hz shaping drops intermediate messages and reports them via a gap frame', async () => {
    await withServer(async ({ nats, port }) => {
      const ws = new WebSocket(`ws://localhost:${port}/ws/v1`);
      await new Promise((resolve) => ws.on('open', resolve));

      // rate_hz: 1 -> at most one forwarded message per second; publishing 3
      // messages back-to-back should forward the first and gap the rest.
      ws.send(JSON.stringify({
        op: 'subscribe',
        streams: [{ type: 'telemetry', vehicle: 'veh-test-3', categories: ['battery'], rate_hz: 1 }],
      }));
      await new Promise((resolve) => setTimeout(resolve, 200));

      const frames = [];
      ws.on('message', (raw) => frames.push(JSON.parse(raw.toString())));

      for (let i = 0; i < 3; i += 1) {
        nats.publish('uaop.telemetry.v1.veh-test-3.battery', sc.encode(`msg-${i}`));
      }
      await new Promise((resolve) => setTimeout(resolve, 300));

      const dataFrames = frames.filter((f) => f.stream === 'telemetry.veh-test-3.battery');
      assert.equal(dataFrames.length, 1); // only the first got through within the 1Hz window
      assert.equal(dataFrames[0].seq, 1);

      ws.close();
    });
  });
});
