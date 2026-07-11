// @test: UAOP-TC-NFR008-E  Metric registry (Prometheus exposition) + health registry aggregation

#include <cassert>
#include <cstdlib>
#include <iostream>

#include <uaop/common/health.h>
#include <uaop/common/metrics.h>

namespace {

using uaop::common::HealthRegistry;
using uaop::common::HealthState;
using uaop::common::MetricRegistry;

void test_counter_and_gauge() {
    MetricRegistry registry;
    auto counter = registry.counter("frames_total", "Total frames processed");
    counter->increment();
    counter->increment(5);
    assert(counter->value() == 6);

    auto gauge = registry.gauge("link_quality", "Current link quality percent");
    gauge->set(87.5);
    assert(gauge->value() == 87.5);
    gauge->add(2.5);
    assert(gauge->value() == 90.0);
}

void test_same_name_labels_returns_same_handle() {
    MetricRegistry registry;
    auto a = registry.counter("x", "help", {{"link", "serial0"}});
    auto b = registry.counter("x", "help", {{"link", "serial0"}});
    assert(a.get() == b.get()); // same underlying Counter, not a duplicate series
    a->increment();
    assert(b->value() == 1);
}

void test_prometheus_render() {
    MetricRegistry registry;
    registry.counter("frames_total", "Total frames", {{"link", "serial0"}})->increment(3);
    registry.gauge("rssi_dbm", "Signal strength")->set(-72.0);

    const std::string out = registry.render_prometheus();
    assert(out.find("# HELP frames_total Total frames") != std::string::npos);
    assert(out.find("# TYPE frames_total counter") != std::string::npos);
    assert(out.find("frames_total{link=\"serial0\"} 3") != std::string::npos);
    assert(out.find("# TYPE rssi_dbm gauge") != std::string::npos);
    assert(out.find("rssi_dbm -72") != std::string::npos);
}

void test_health_aggregation() {
    HealthRegistry health;
    assert(health.overall() == HealthState::kOk); // nothing registered yet

    health.set("nats", HealthState::kOk);
    assert(health.overall() == HealthState::kOk);

    health.set("db", HealthState::kDegraded, "connection pool at 90%");
    assert(health.overall() == HealthState::kDegraded); // worst-of

    health.set("bridge", HealthState::kFail, "link down");
    assert(health.overall() == HealthState::kFail); // FAIL dominates DEGRADED

    health.set("bridge", HealthState::kOk); // recovers
    assert(health.overall() == HealthState::kDegraded); // db still degraded
}

void test_health_json() {
    HealthRegistry health;
    health.set("nats", HealthState::kFail, "connection refused");
    const std::string json = health.render_json();
    assert(json.find("\"status\":\"FAIL\"") != std::string::npos);
    assert(json.find("\"nats\":{\"state\":\"FAIL\",\"reason\":\"connection refused\"}") != std::string::npos);
}

} // namespace

int main() {
    test_counter_and_gauge();
    test_same_name_labels_returns_same_handle();
    test_prometheus_render();
    test_health_aggregation();
    test_health_json();
    std::cout << "metrics-health-tests: all assertions passed\n";
    return EXIT_SUCCESS;
}
