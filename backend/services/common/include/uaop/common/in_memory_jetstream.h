// uaop::common::InMemoryJetStream — a real, tested JetStreamPublisher +
// JetStreamConsumer adapter for single-process use: services/common's own
// tests, and any service developing against the JetStream port before the
// real NATS adapter lands (jetstream.h's header comment has the status).
// Durability is scoped to this object's lifetime only — "durable" here means
// "the consumer's ack position is tracked," not "survives a process
// restart," which is the one respect in which this is not a drop-in
// stand-in for the real adapter. Subject matching is exact-match only (no
// NATS wildcard subjects) — sufficient for tests, explicitly not a feature.

#ifndef UAOP_COMMON_IN_MEMORY_JETSTREAM_H
#define UAOP_COMMON_IN_MEMORY_JETSTREAM_H

#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include <uaop/common/jetstream.h>

namespace uaop::common {

class InMemoryJetStream : public JetStreamPublisher, public JetStreamConsumer {
public:
    [[nodiscard]] Result<Unit, Error> publish(const std::string& subject, std::vector<uint8_t> payload) override;

    [[nodiscard]] Result<Unit, Error> subscribe(const std::string& subject, const std::string& durable_name,
                                                 std::function<void(const JetStreamMessage&)> handler) override;

    void ack(uint64_t stream_sequence) override;

    // Test hook: re-delivers an already-published message to every current
    // subscriber on its subject, simulating JetStream's at-least-once
    // redelivery (e.g. an unacked message after a consumer restart).
    void redeliver(uint64_t stream_sequence);

    [[nodiscard]] uint64_t last_published_sequence() const;
    [[nodiscard]] uint64_t last_acked_sequence() const;

private:
    struct Subscription {
        std::string subject;
        std::string durable_name;
        std::function<void(const JetStreamMessage&)> handler;
    };

    mutable std::mutex mutex_;
    uint64_t next_sequence_ = 1;
    uint64_t last_acked_ = 0;
    std::map<uint64_t, JetStreamMessage> log_; // stream_sequence -> message, for redeliver()
    std::vector<Subscription> subscriptions_;
};

} // namespace uaop::common

#endif // UAOP_COMMON_IN_MEMORY_JETSTREAM_H
