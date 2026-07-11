// uaop::common::MetricRegistry — counters and gauges, Prometheus text-exposition
// format (OBSERVABILITY.md; "how much", as distinct from logs' "why" —
// LOGGING.md §6). Handles are stable references the caller holds and updates
// with a lock-free atomic op — registration (name lookup, allocation) happens
// once at startup, never on a hot path. This is what makes it safe for the
// "periodic flushed counters" exception to the RT no-logging rule
// (LOGGING.md §2, ADR-0013): the counter itself never allocates or locks.

#ifndef UAOP_COMMON_METRICS_H
#define UAOP_COMMON_METRICS_H

#include <atomic>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace uaop::common {

using Labels = std::map<std::string, std::string>; // ordered: stable Prometheus output

class Counter {
public:
    void increment(uint64_t delta = 1) noexcept { value_.fetch_add(delta, std::memory_order_relaxed); }
    [[nodiscard]] uint64_t value() const noexcept { return value_.load(std::memory_order_relaxed); }

private:
    std::atomic<uint64_t> value_{0};
};

class Gauge {
public:
    void set(double v) noexcept { bits_.store(to_bits(v), std::memory_order_relaxed); }
    void add(double delta) noexcept { set(value() + delta); } // read-modify-write, not lock-free vs concurrent add
    [[nodiscard]] double value() const noexcept { return from_bits(bits_.load(std::memory_order_relaxed)); }

private:
    static uint64_t to_bits(double v) noexcept;
    static double from_bits(uint64_t b) noexcept;

    std::atomic<uint64_t> bits_{0};
};

// A registered metric plus the metadata Prometheus exposition needs.
struct MetricEntry {
    std::string name;
    std::string help;
    Labels labels;
    enum class Kind { kCounter, kGauge } kind;
    std::shared_ptr<Counter> counter; // set iff kind == kCounter
    std::shared_ptr<Gauge> gauge;     // set iff kind == kGauge
};

class MetricRegistry {
public:
    // Same (name, labels) pair returns the same handle on repeated calls —
    // safe to call from initialization code that runs more than once (e.g.
    // per-connection setup) without accumulating duplicate series.
    [[nodiscard]] std::shared_ptr<Counter> counter(const std::string& name, const std::string& help,
                                                    Labels labels = {});
    [[nodiscard]] std::shared_ptr<Gauge> gauge(const std::string& name, const std::string& help,
                                                Labels labels = {});

    // Prometheus text exposition format (https://prometheus.io/docs/instrumenting/exposition_formats/).
    [[nodiscard]] std::string render_prometheus() const;

private:
    [[nodiscard]] std::string key(const std::string& name, const Labels& labels) const;

    mutable std::mutex mutex_;
    std::vector<MetricEntry> entries_;
    std::map<std::string, std::size_t> index_; // key() -> entries_ position
};

} // namespace uaop::common

#endif // UAOP_COMMON_METRICS_H
