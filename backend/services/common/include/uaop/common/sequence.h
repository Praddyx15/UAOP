// uaop::common::SequenceCounter — per-key monotonic sequence numbers, shared by
// TelemetryEnvelope.sequence (TELEMETRY_ENGINE.md §5, per-vehicle) and
// EventEnvelope.sequence (EVENT_FLOW.md §4, per-source). One counter type, two
// callers — the gap-detection/dedup semantics both envelopes rely on come from
// this being genuinely monotonic and thread-safe, not from each call site
// reimplementing its own atomic.
//
// @req: UAOP-NFR-008  No exceptions on real-time paths; Result<T,E> pattern platform-wide

#ifndef UAOP_COMMON_SEQUENCE_H
#define UAOP_COMMON_SEQUENCE_H

#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>

namespace uaop::common {

// Single-key monotonic counter. next() is lock-free; starts at 1 (0 is
// reserved to mean "no sequence assigned yet" at call sites that need that).
class SequenceCounter {
public:
    [[nodiscard]] uint64_t next() noexcept { return counter_.fetch_add(1, std::memory_order_relaxed) + 1; }

    [[nodiscard]] uint64_t current() const noexcept { return counter_.load(std::memory_order_relaxed); }

private:
    std::atomic<uint64_t> counter_{0};
};

// Per-key registry of SequenceCounter (key = vehicle_id or source_service).
// Each key's counter is independent and monotonic from the point it's first
// touched — restart resets to 1, which is why gap detection keys off observed
// sequence discontinuities rather than assuming a global start point.
class KeyedSequenceCounter {
public:
    [[nodiscard]] uint64_t next(const std::string& key) {
        std::lock_guard<std::mutex> lock(mutex_);
        return counters_[key].next();
    }

private:
    std::mutex mutex_;
    std::unordered_map<std::string, SequenceCounter> counters_;
};

} // namespace uaop::common

#endif // UAOP_COMMON_SEQUENCE_H
