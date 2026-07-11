// uaop::common::DedupSet — in-memory TTL dedup set, the same shape as the
// Redis-backed dedup DATABASE.md §6 describes ("dedup sets use TTL matching
// the JetStream redelivery horizon"). This in-memory version is what a
// single-process consumer uses directly; a Redis-backed adapter with the same
// insert()/contains() surface is a straightforward follow-up once a service
// actually needs cross-process dedup (Redis client wiring is its own
// dependency decision, out of scope here).

#ifndef UAOP_COMMON_DEDUP_H
#define UAOP_COMMON_DEDUP_H

#include <chrono>
#include <mutex>
#include <string>
#include <unordered_map>

namespace uaop::common {

class DedupSet {
public:
    explicit DedupSet(std::chrono::milliseconds ttl) : ttl_(ttl) {}

    // Returns true if `key` was newly inserted (i.e. this delivery is novel);
    // false if it was already present and unexpired (i.e. this is a
    // duplicate — the caller should skip processing it). Expired entries are
    // swept opportunistically on each call, bounding memory without a
    // separate background thread.
    [[nodiscard]] bool insert(const std::string& key) {
        const auto now = std::chrono::steady_clock::now();
        std::lock_guard<std::mutex> lock(mutex_);
        sweep(now);
        auto [it, inserted] = entries_.try_emplace(key, now + ttl_);
        if (!inserted) {
            return false; // already present and (by sweep) still unexpired
        }
        return true;
    }

    [[nodiscard]] std::size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return entries_.size();
    }

private:
    void sweep(std::chrono::steady_clock::time_point now) {
        for (auto it = entries_.begin(); it != entries_.end();) {
            it = (it->second <= now) ? entries_.erase(it) : std::next(it);
        }
    }

    std::chrono::milliseconds ttl_;
    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::chrono::steady_clock::time_point> entries_;
};

} // namespace uaop::common

#endif // UAOP_COMMON_DEDUP_H
