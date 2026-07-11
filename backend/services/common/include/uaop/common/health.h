// uaop::common::HealthRegistry — named health checks, aggregated into one
// overall status for a service's /healthz. Push model: call sites (a NATS
// consumer, a DB pool, a link supervisor) call set() whenever their own state
// changes; nothing here re-invokes arbitrary callbacks, so there's no risk of
// a health check blocking on a lock the caller already holds.
//
// DEPLOYMENT.md §6: the deployment smoke suite checks "service
// health/readiness across namespaces" — this registry is what a service's
// /healthz endpoint (health_metrics_server.h) reports from.

#ifndef UAOP_COMMON_HEALTH_H
#define UAOP_COMMON_HEALTH_H

#include <map>
#include <mutex>
#include <string>

namespace uaop::common {

enum class HealthState : int { kOk = 0, kDegraded = 1, kFail = 2 };

inline const char* to_string(HealthState s) noexcept {
    switch (s) {
        case HealthState::kOk: return "OK";
        case HealthState::kDegraded: return "DEGRADED";
        case HealthState::kFail: return "FAIL";
    }
    return "UNKNOWN";
}

class HealthRegistry {
public:
    void set(const std::string& check_name, HealthState state, std::string reason = {});

    // Worst state across all registered checks; kOk if none are registered
    // yet (a service that hasn't finished starting reports FAIL explicitly
    // via its own startup check, not by relying on an empty registry).
    [[nodiscard]] HealthState overall() const;

    // {"status":"OK","checks":{"nats":{"state":"OK","reason":""},...}}
    [[nodiscard]] std::string render_json() const;

private:
    struct Check {
        HealthState state;
        std::string reason;
    };

    mutable std::mutex mutex_;
    std::map<std::string, Check> checks_;
};

} // namespace uaop::common

#endif // UAOP_COMMON_HEALTH_H
