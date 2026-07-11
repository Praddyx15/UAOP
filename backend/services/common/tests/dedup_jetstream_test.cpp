// @test: UAOP-TC-NFR008-F  DedupSet TTL behavior + InMemoryJetStream publish/subscribe/redeliver,
// composed to demonstrate idempotent consumption (EVENT_FLOW.md §3) at unit-test scope.

#include <cassert>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <vector>

#include <uaop/common/dedup.h>
#include <uaop/common/in_memory_jetstream.h>

namespace {

using uaop::common::DedupSet;
using uaop::common::InMemoryJetStream;
using uaop::common::JetStreamMessage;

void test_dedup_basic() {
    DedupSet dedup(std::chrono::milliseconds(5000));
    assert(dedup.insert("evt-1") == true);  // novel
    assert(dedup.insert("evt-1") == false); // duplicate
    assert(dedup.insert("evt-2") == true);  // different key, novel
    assert(dedup.size() == 2);
}

void test_dedup_ttl_expiry() {
    DedupSet dedup(std::chrono::milliseconds(20));
    assert(dedup.insert("evt-1") == true);
    std::this_thread::sleep_for(std::chrono::milliseconds(60));
    assert(dedup.insert("evt-1") == true); // expired -> novel again
}

void test_jetstream_publish_subscribe() {
    InMemoryJetStream js;
    std::vector<std::string> received;
    auto sub = js.subscribe("uaop.telemetry.v1.veh-01.position", "telemetry-engine",
                             [&received](const JetStreamMessage& msg) {
                                 received.emplace_back(msg.payload.begin(), msg.payload.end());
                             });
    assert(sub.ok());

    auto pub = js.publish("uaop.telemetry.v1.veh-01.position", {'p', 'o', 's', '1'});
    assert(pub.ok());
    assert(received.size() == 1);
    assert(received[0] == "pos1");
    assert(js.last_published_sequence() == 1);

    js.ack(js.last_published_sequence());
    assert(js.last_acked_sequence() == 1);
}

void test_jetstream_redelivery_deduped_by_caller() {
    // Simulates the real integration pattern: JetStream (or here, its stub)
    // guarantees at-least-once; the consumer's own DedupSet is what makes
    // redelivery a no-op instead of double-processing.
    InMemoryJetStream js;
    DedupSet dedup(std::chrono::milliseconds(5000));
    int processed_count = 0;

    auto sub = js.subscribe("uaop.event.v1.vehicle.state_changed", "vehicle-manager",
                             [&](const JetStreamMessage& msg) {
                                 const std::string event_id(msg.payload.begin(), msg.payload.end());
                                 if (dedup.insert(event_id)) {
                                     ++processed_count; // only novel deliveries count
                                 }
                             });
    assert(sub.ok());

    auto pub1 = js.publish("uaop.event.v1.vehicle.state_changed", {'e', 'v', 't', '-', '1'});
    assert(pub1.ok());
    assert(processed_count == 1);

    js.redeliver(js.last_published_sequence()); // at-least-once redelivery
    assert(processed_count == 1); // deduped — still 1, not 2

    auto pub2 = js.publish("uaop.event.v1.vehicle.state_changed", {'e', 'v', 't', '-', '2'});
    assert(pub2.ok());
    assert(processed_count == 2); // genuinely new event still counted
}

void test_jetstream_exact_subject_no_wildcard() {
    InMemoryJetStream js;
    int hits = 0;
    auto sub = js.subscribe("uaop.telemetry.v1.veh-01.position", "d1", [&](const JetStreamMessage&) { ++hits; });
    assert(sub.ok());
    auto pub = js.publish("uaop.telemetry.v1.veh-01.attitude", {'x'}); // different subject
    assert(pub.ok());
    assert(hits == 0); // exact-match only, documented limitation
}

} // namespace

int main() {
    test_dedup_basic();
    test_dedup_ttl_expiry();
    test_jetstream_publish_subscribe();
    test_jetstream_redelivery_deduped_by_caller();
    test_jetstream_exact_subject_no_wildcard();
    std::cout << "dedup-jetstream-tests: all assertions passed\n";
    return EXIT_SUCCESS;
}
