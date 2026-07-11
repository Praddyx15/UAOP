// uaop::common::JetStreamPublisher / JetStreamConsumer — the NATS JetStream
// port (hexagonal pattern established in tools/service-template's
// EventPublisher/StdoutPublisher: domain code depends on this interface, not
// a concrete client). EVENT_FLOW.md §3: at-least-once delivery everywhere,
// ordering guaranteed per-subject only, durable consumers resume at the ack
// floor after a restart — this interface's shape exists to make those
// guarantees representable, not to hide them.
//
// Real-adapter status (2026-07-07, M1.4): no adapter here talks to an actual
// NATS server yet. Vendoring nats.c (the official C client JetStream needs)
// is real, unverified work — cross-platform build, OpenSSL linkage, MinGW
// quirks — that deserves its own timeboxed spike rather than a blind attempt
// bundled into this task, the same judgment call ADR-0020 made for MapLibre
// Native Qt. InMemoryJetStream (in_memory_jetstream.h) is a real, tested
// adapter for single-process development and for this library's own tests;
// api-gateway v0 (M1.6) and the stream-provisioning script (M1.5) talk to the
// real NATS server directly instead of through this C++ port, so M1 is not
// blocked on the spike landing first.

#ifndef UAOP_COMMON_JETSTREAM_H
#define UAOP_COMMON_JETSTREAM_H

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include <uaop/common/error.h>
#include <uaop/common/result.h>

namespace uaop::common {

struct JetStreamMessage {
    std::string subject;
    std::vector<uint8_t> payload;
    uint64_t stream_sequence = 0; // JetStream's own delivery sequence — what ack() takes
};

class JetStreamPublisher {
public:
    virtual ~JetStreamPublisher() = default;
    [[nodiscard]] virtual Result<Unit, Error> publish(const std::string& subject,
                                                        std::vector<uint8_t> payload) = 0;
};

// Durable: `durable_name` identifies a consumer whose position survives
// process restart (on a real adapter — InMemoryJetStream's durability is
// scoped to its own lifetime, documented on that class). Handler runs
// at-least-once per message; callers dedup via DedupSet (dedup.h) keyed on
// their own idempotency key (event_id for events, (vehicle_id, sequence) for
// telemetry — EVENT_FLOW.md §3), not on stream_sequence, which is a delivery
// position, not a content identity.
class JetStreamConsumer {
public:
    virtual ~JetStreamConsumer() = default;
    [[nodiscard]] virtual Result<Unit, Error> subscribe(
        const std::string& subject, const std::string& durable_name,
        std::function<void(const JetStreamMessage&)> handler) = 0;
    virtual void ack(uint64_t stream_sequence) = 0;
};

} // namespace uaop::common

#endif // UAOP_COMMON_JETSTREAM_H
