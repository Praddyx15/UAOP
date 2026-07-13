#!/usr/bin/env bash
# Provisions the five platform JetStream streams (EVENT_FLOW.md §2) against a
# running NATS server. Idempotent: `nats stream add` is skipped (not errored)
# for a stream that already exists with the same config.
#
# Usage: tools/provision-streams.sh [nats-server-url]   (default nats://localhost:4222)
#
# AUDIT fsync-on-write (ADR-0015) — documented gap: the nats CLI's
# --persist-mode flag only accepts "default"/"async" (verified via
# `nats stream add --help` against nats CLI 0.4.0 / nats-server 2.10), neither
# of which is a true always-fsync mode. True fsync-on-write requires a
# server-level JetStream `sync_interval` setting (nats-server config file),
# not a per-stream CLI flag — the workstation compose profile currently
# starts nats-server with bare CLI flags (`-js -sd /data -m 8222`, no config
# file). Tracked as follow-up work, not solved by this script.

set -euo pipefail

NATS_URL="${1:-nats://localhost:4222}"

echo "[provision-streams] target: ${NATS_URL}"

add_stream() {
  local name="$1"
  shift
  if nats --server "${NATS_URL}" stream info "${name}" >/dev/null 2>&1; then
    echo "[provision-streams] ${name}: already exists, skipping"
    return 0
  fi
  echo "[provision-streams] ${name}: creating"
  nats --server "${NATS_URL}" stream add "${name}" "$@" --defaults
}

# TELEMETRY: 24h / interest-based retention (consumers: telemetry-engine
# durable, gateway ephemeral, ai-engine durable Ph2) — EVENT_FLOW.md §2.
add_stream TELEMETRY \
  --subjects="uaop.telemetry.v1.>" \
  --storage=file \
  --retention=interest \
  --max-age=24h \
  --max-bytes=10GB \
  --discard=old

# EVENTS: 30d, all domain consumers durable.
add_stream EVENTS \
  --subjects="uaop.event.v1.>" \
  --storage=file \
  --retention=limits \
  --max-age=30d \
  --max-bytes=5GB \
  --discard=old

# COMMANDS: work-queue to vehicle-manager — messages remove on ack, not age.
add_stream COMMANDS \
  --subjects="uaop.cmd.v1.>" \
  --storage=file \
  --retention=work \
  --max-bytes=1GB \
  --discard=old

# AI: 30d.
add_stream AI \
  --subjects="uaop.ai.v1.>" \
  --storage=file \
  --retention=limits \
  --max-age=30d \
  --max-bytes=2GB \
  --discard=old

# AUDIT: single persistent intake stream, never truncated before
# compliance-engine ack + chain append — no age limit, generous size cap as a
# safety net rather than a real retention policy (this stream isn't meant to
# fill under normal operation; the Postgres chain is the durable long-term
# home, per COMPLIANCE.md §B.3).
add_stream AUDIT \
  --subjects="uaop.audit.v1" \
  --storage=file \
  --retention=limits \
  --max-bytes=20GB \
  --discard=old

echo "[provision-streams] done"
nats --server "${NATS_URL}" stream ls
