#include <uaop/common/in_memory_jetstream.h>

namespace uaop::common {

Result<Unit, Error> InMemoryJetStream::publish(const std::string& subject, std::vector<uint8_t> payload) {
    std::vector<Subscription> to_notify;
    JetStreamMessage msg;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        msg.subject = subject;
        msg.payload = std::move(payload);
        msg.stream_sequence = next_sequence_++;
        log_[msg.stream_sequence] = msg;
        for (const auto& sub : subscriptions_) {
            if (sub.subject == subject) {
                to_notify.push_back(sub);
            }
        }
    }
    for (const auto& sub : to_notify) {
        sub.handler(msg);
    }
    return Result<Unit, Error>::Ok(kUnit);
}

Result<Unit, Error> InMemoryJetStream::subscribe(const std::string& subject, const std::string& durable_name,
                                                  std::function<void(const JetStreamMessage&)> handler) {
    std::lock_guard<std::mutex> lock(mutex_);
    subscriptions_.push_back(Subscription{subject, durable_name, std::move(handler)});
    return Result<Unit, Error>::Ok(kUnit);
}

void InMemoryJetStream::ack(uint64_t stream_sequence) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (stream_sequence > last_acked_) {
        last_acked_ = stream_sequence;
    }
}

void InMemoryJetStream::redeliver(uint64_t stream_sequence) {
    std::vector<Subscription> to_notify;
    JetStreamMessage msg;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = log_.find(stream_sequence);
        if (it == log_.end()) {
            return;
        }
        msg = it->second;
        for (const auto& sub : subscriptions_) {
            if (sub.subject == msg.subject) {
                to_notify.push_back(sub);
            }
        }
    }
    for (const auto& sub : to_notify) {
        sub.handler(msg);
    }
}

uint64_t InMemoryJetStream::last_published_sequence() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return next_sequence_ - 1;
}

uint64_t InMemoryJetStream::last_acked_sequence() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return last_acked_;
}

} // namespace uaop::common
