import http from 'node:http';

import express from 'express';
import Redis from 'ioredis';
import { connect as natsConnect } from 'nats';

import { config } from './config.js';
import { Denylist } from './auth/denylist.js';
import { authenticate } from './auth/middleware.js';
import { authRouter } from './routes/auth.js';
import { healthRouter } from './routes/health.js';
import { commandsRouter } from './routes/commands.js';
import { createStreamServer } from './streams/websocket.js';

export async function startGateway() {
  const redis = new Redis(config.redisUrl);
  const denylist = new Denylist(redis);
  const nats = await natsConnect({ servers: config.natsUrl });

  const app = express();
  app.use(express.json());
  app.use(healthRouter());
  app.use(authRouter({ denylist }));
  app.use(authenticate(denylist));
  app.use(commandsRouter());

  const server = http.createServer(app);
  createStreamServer({ server, natsConnection: nats, heartbeatMs: config.heartbeatMs });

  await new Promise((resolve) => server.listen(config.httpPort, resolve));
  // eslint-disable-next-line no-console
  console.log(`[api-gateway] listening on :${config.httpPort} (WS on the same port at /ws/v1)`);

  return {
    server,
    nats,
    redis,
    async close() {
      await new Promise((resolve) => server.close(resolve));
      await nats.close();
      redis.disconnect();
    },
  };
}

if (import.meta.url === `file://${process.argv[1]}`) {
  startGateway().catch((err) => {
    // eslint-disable-next-line no-console
    console.error('[api-gateway] failed to start:', err);
    process.exit(1);
  });
}
