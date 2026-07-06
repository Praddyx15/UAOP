// Domain: per-vehicle link supervision from heartbeat arrivals. Pure C++17 — no framework,
// no I/O, no allocation in the per-sample path. TEMPLATE code: placeholder trace IDs,
// excluded from RTM scan (see README.md).
//
// @req: UAOP-TPL-000  Template placeholder — replace with real requirement ID when copied

#ifndef UAOP_TEMPLATE_DOMAIN_LINK_MONITOR_H
#define UAOP_TEMPLATE_DOMAIN_LINK_MONITOR_H

#include <cstdint>

#include <uaop/common/error.h>
#include <uaop/common/result.h>

namespace uaop::tpl::domain {

enum class LinkState : std::uint8_t { kNeverSeen = 0, kAlive = 1, kLost = 2 };

struct Transition {
    LinkState from;
    LinkState to;
    bool changed;
};

// Tracks heartbeat recency against a timeout. Monotonic time in — LinkState out.
// The caller owns the clock (a port); the domain never reads time itself.
class LinkMonitor {
public:
    explicit LinkMonitor(std::uint64_t timeout_ms) : timeout_ms_(timeout_ms) {}

    // Records a heartbeat at now_ms. Rejects non-monotonic input rather than
    // silently corrupting recency state (no silent degradation).
    common::Result<Transition, common::Error> on_heartbeat(std::uint64_t now_ms) {
        using R = common::Result<Transition, common::Error>;
        if (state_ != LinkState::kNeverSeen && now_ms < last_seen_ms_) {
            return R::Err(common::Error::Invalid("E_TIME_REGRESSION",
                                                 "heartbeat timestamp precedes last seen"));
        }
        last_seen_ms_ = now_ms;
        return R::Ok(transition_to(LinkState::kAlive));
    }

    // Evaluates timeout at now_ms; called periodically by the app layer.
    Transition on_tick(std::uint64_t now_ms) {
        if (state_ == LinkState::kAlive && now_ms - last_seen_ms_ > timeout_ms_) {
            return transition_to(LinkState::kLost);
        }
        return Transition{state_, state_, false};
    }

    [[nodiscard]] LinkState state() const noexcept { return state_; }

private:
    Transition transition_to(LinkState next) {
        const Transition t{state_, next, state_ != next};
        state_ = next;
        return t;
    }

    std::uint64_t timeout_ms_;
    std::uint64_t last_seen_ms_ = 0;
    LinkState state_ = LinkState::kNeverSeen;
};

} // namespace uaop::tpl::domain

#endif // UAOP_TEMPLATE_DOMAIN_LINK_MONITOR_H
