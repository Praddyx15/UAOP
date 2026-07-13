// WebSocket subscription protocol (API_SPECIFICATION.md §5). Client sends one
// `subscribe` op naming telemetry/event streams; server relays NATS messages
// on the matching subjects as `{stream, seq, ts, payload}` frames. Per-stream
// `seq` is assigned by the gateway itself (a WS-session-local counter, not
// the source's own sequence) — it's what lets the client detect gateway-side
// shedding via the explicit `gap` frame, independent of whatever
// gap-accounting the source telemetry stream itself does (TELEMETRY_ENGINE.md
// §5 is a different, source-side concept).

import { WebSocketServer } from 'ws';

function telemetrySubject(vehicle, category) {
  return `uaop.telemetry.v1.${vehicle}.${category}`;
}

function eventSubject(domain) {
  return `uaop.event.v1.${domain}.>`;
}

class StreamState {
  constructor(key, rateHz) {
    this.key = key;
    this.seq = 0;
    this.dropped = 0;
    this.minIntervalMs = rateHz && rateHz > 0 ? 1000 / rateHz : 0;
    this.lastForwardedAt = 0;
  }

  // Returns true if this message should be forwarded now (rate_hz shaping);
  // false means it counts toward the next gap frame instead.
  shouldForward(nowMs) {
    if (this.minIntervalMs === 0) {
      return true;
    }
    if (nowMs - this.lastForwardedAt >= this.minIntervalMs) {
      this.lastForwardedAt = nowMs;
      return true;
    }
    this.dropped += 1;
    return false;
  }
}

class ClientSession {
  constructor(ws, natsConnection, heartbeatMs) {
    this.ws = ws;
    this.nats = natsConnection;
    this.subs = []; // NATS subscription handles, for cleanup
    this.streams = new Map(); // stream key -> StreamState

    this.heartbeatTimer = setInterval(() => this.sendHeartbeat(), heartbeatMs);

    ws.on('message', (raw) => this.handleMessage(raw));
    ws.on('close', () => this.cleanup());
  }

  send(frame) {
    if (this.ws.readyState === this.ws.OPEN) {
      this.ws.send(JSON.stringify(frame));
    }
  }

  sendHeartbeat() {
    this.send({ op: 'heartbeat', ts: new Date().toISOString() });
  }

  async handleMessage(raw) {
    let msg;
    try {
      msg = JSON.parse(raw.toString());
    } catch {
      this.send({ op: 'error', message: 'invalid JSON' });
      return;
    }
    if (msg.op === 'subscribe') {
      await this.handleSubscribe(msg.streams ?? []);
    } else {
      this.send({ op: 'error', message: `unknown op: ${msg.op}` });
    }
  }

  async handleSubscribe(streamRequests) {
    // Resubscription is cheap and stateless (API_SPECIFICATION.md §5):
    // tear down any prior subscriptions before establishing the new set.
    await this.cleanupSubscriptions();

    for (const req of streamRequests) {
      if (req.type === 'telemetry') {
        for (const category of req.categories ?? []) {
          const streamKey = `telemetry.${req.vehicle}.${category}`;
          const state = new StreamState(streamKey, req.rate_hz);
          this.streams.set(streamKey, state);
          const sub = this.nats.subscribe(telemetrySubject(req.vehicle, category));
          this.subs.push(sub);
          this.pump(sub, state);
        }
      } else if (req.type === 'events') {
        for (const domain of req.domains ?? []) {
          const streamKey = `events.${domain}`;
          const state = new StreamState(streamKey, null); // events are not rate-shaped
          this.streams.set(streamKey, state);
          const sub = this.nats.subscribe(eventSubject(domain));
          this.subs.push(sub);
          this.pump(sub, state);
        }
      }
    }
  }

  // Drains one NATS subscription's async iterator into WS frames, applying
  // this stream's rate shaping and gap accounting.
  async pump(sub, state) {
    for await (const msg of sub) {
      const now = Date.now();
      if (!state.shouldForward(now)) {
        continue;
      }
      if (state.dropped > 0) {
        this.send({ op: 'gap', stream: state.key, dropped: state.dropped });
        state.dropped = 0;
      }
      state.seq += 1;
      this.send({
        stream: state.key,
        seq: state.seq,
        ts: new Date().toISOString(),
        payload: Buffer.from(msg.data).toString('base64'),
      });
    }
  }

  async cleanupSubscriptions() {
    for (const sub of this.subs) {
      sub.unsubscribe();
    }
    this.subs = [];
    this.streams.clear();
  }

  cleanup() {
    clearInterval(this.heartbeatTimer);
    this.cleanupSubscriptions();
  }
}

export function createStreamServer({ server, natsConnection, heartbeatMs }) {
  const wss = new WebSocketServer({ server, path: '/ws/v1' });
  wss.on('connection', (ws) => new ClientSession(ws, natsConnection, heartbeatMs));
  return wss;
}
