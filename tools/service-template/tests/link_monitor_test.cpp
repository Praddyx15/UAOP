// Domain tests — no I/O, no adapters, the pattern every real service follows.
// @test: UAOP-TPL-000  Template placeholder

#include <cassert>
#include <cstdlib>
#include <iostream>

#include "domain/link_monitor.h"

namespace {

using uaop::tpl::domain::LinkMonitor;
using uaop::tpl::domain::LinkState;

void test_alive_on_first_heartbeat() {
    LinkMonitor m(3000);
    assert(m.state() == LinkState::kNeverSeen);
    const auto r = m.on_heartbeat(1000);
    assert(r.ok());
    assert(r.value().changed);
    assert(m.state() == LinkState::kAlive);
}

void test_lost_after_timeout_exactly_once() {
    LinkMonitor m(3000);
    (void)m.on_heartbeat(1000);
    assert(!m.on_tick(3999).changed); // 2999 ms elapsed: within timeout
    const auto lost = m.on_tick(4001); // 3001 ms elapsed: past timeout
    assert(lost.changed);
    assert(lost.to == LinkState::kLost);
    assert(!m.on_tick(9000).changed); // already lost: no repeat transition
}

void test_recovery() {
    LinkMonitor m(3000);
    (void)m.on_heartbeat(0);
    (void)m.on_tick(5000); // lost
    const auto r = m.on_heartbeat(6000);
    assert(r.ok());
    assert(r.value().changed);
    assert(r.value().to == LinkState::kAlive);
}

void test_time_regression_rejected() {
    LinkMonitor m(3000);
    (void)m.on_heartbeat(5000);
    const auto r = m.on_heartbeat(4000);
    assert(!r.ok());
    assert(r.error().code() == "E_TIME_REGRESSION");
    assert(m.state() == LinkState::kAlive); // state uncorrupted by bad input
}

} // namespace

int main() {
    test_alive_on_first_heartbeat();
    test_lost_after_timeout_exactly_once();
    test_recovery();
    test_time_regression_rejected();
    std::cout << "uaop-service-template-tests: all assertions passed\n";
    return EXIT_SUCCESS;
}
