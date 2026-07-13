# api-gateway v0 (ADR-0004). Multi-stage: install deps in a full node image,
# run in a slim runtime image — same spirit as the distroless-runtime pattern
# other services follow (PROJECT_STRUCTURE.md §3), scoped to what's actually
# available for Node (a slim base, since a fully distroless Node image would
# still need the Node runtime layer itself).
#
# Build context must be the repo root (this service loads .proto files from
# api/proto/ at runtime via @grpc/proto-loader, which lives outside a
# per-service build context):
#   docker build -f backend/api-gateway/Dockerfile -t uaop/api-gateway .

FROM node:22-slim AS deps
WORKDIR /app
COPY backend/api-gateway/package.json ./
RUN npm install --omit=dev

FROM node:22-slim AS runtime
WORKDIR /app
ENV NODE_ENV=production
COPY --from=deps /app/node_modules ./node_modules
COPY backend/api-gateway/package.json ./
COPY backend/api-gateway/src ./src
COPY api/proto /api/proto
EXPOSE 8080
USER node
CMD ["node", "src/index.js"]
