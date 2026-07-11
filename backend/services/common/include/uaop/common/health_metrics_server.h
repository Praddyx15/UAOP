// uaop::common::HealthMetricsServer — the minimal `GET /healthz` + `GET /metrics`
// listener every service exposes (SOFTWARE_ARCHITECTURE.md §5's "health/metrics
// endpoints" line item). Deliberately not Drogon: Drogon is the gRPC/REST
// framework services use for their real API surface once they exist
// (CLAUDE.md stack lock), but no service exists yet to pull it in, and
// health/metrics is a tiny, dependency-free surface on its own — pulling in a
// full HTTP framework just to serve two GET routes would be exactly the kind
// of premature dependency this project's own review discipline flags.
// Sequential connection handling (no thread-per-connection): this endpoint is
// scraped periodically, not a hot path, and simplicity here is a feature.

#ifndef UAOP_COMMON_HEALTH_METRICS_SERVER_H
#define UAOP_COMMON_HEALTH_METRICS_SERVER_H

#include <atomic>
#include <cstdint>
#include <thread>

#include <uaop/common/health.h>
#include <uaop/common/metrics.h>

namespace uaop::common {

class HealthMetricsServer {
public:
    HealthMetricsServer(HealthRegistry& health, MetricRegistry& metrics);
    ~HealthMetricsServer();

    HealthMetricsServer(const HealthMetricsServer&) = delete;
    HealthMetricsServer& operator=(const HealthMetricsServer&) = delete;

    // Binds to 0.0.0.0:port and starts the accept loop on a background
    // thread. Returns false if the bind/listen failed (port in use, etc.) —
    // never throws (ADR-0013 posture even off the RT path, for consistency).
    [[nodiscard]] bool start(uint16_t port);

    // Idempotent; safe to call from the destructor path.
    void stop();

    [[nodiscard]] uint16_t port() const noexcept { return port_; }

private:
    void run();
    void handle_connection(int64_t client_fd);

    HealthRegistry& health_;
    MetricRegistry& metrics_;
    uint16_t port_ = 0;
    int64_t listen_fd_ = -1; // holds a real socket handle once started (widened for portability)
    std::atomic<bool> running_{false};
    std::thread accept_thread_;
};

} // namespace uaop::common

#endif // UAOP_COMMON_HEALTH_METRICS_SERVER_H
