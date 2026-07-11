// @test: UAOP-TC-NFR008-B  UUIDv7 format/monotonicity, SequenceCounter behavior

#include <cassert>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <set>
#include <thread>

#include <uaop/common/sequence.h>
#include <uaop/common/uuid.h>

namespace {

using uaop::common::KeyedSequenceCounter;
using uaop::common::SequenceCounter;
using uaop::common::uuid_v7;

void test_uuid_format() {
    const std::string id = uuid_v7();
    assert(id.size() == 36);
    assert(id[8] == '-' && id[13] == '-' && id[18] == '-' && id[23] == '-');
    assert(id[14] == '7'); // version nibble
    const char variant = id[19];
    assert(variant == '8' || variant == '9' || variant == 'a' || variant == 'b');
}

void test_uuid_uniqueness() {
    std::set<std::string> seen;
    for (int i = 0; i < 1000; ++i) {
        assert(seen.insert(uuid_v7()).second);
    }
}

void test_uuid_time_ordering() {
    // Ordering is only guaranteed across distinct milliseconds — the 48-bit
    // timestamp dominates the leading hex digits, but two IDs sharing a
    // millisecond are ordered by random bits, not guaranteed non-decreasing
    // (that's the "v7 vs a real counter" tradeoff). Force a millisecond gap
    // so this test asserts what UUIDv7 actually promises.
    const std::string a = uuid_v7();
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    const std::string b = uuid_v7();
    assert(a < b);
}

void test_sequence_counter() {
    SequenceCounter c;
    assert(c.current() == 0);
    assert(c.next() == 1);
    assert(c.next() == 2);
    assert(c.current() == 2);
}

void test_keyed_sequence_counter() {
    KeyedSequenceCounter kc;
    assert(kc.next("veh-01") == 1);
    assert(kc.next("veh-01") == 2);
    assert(kc.next("veh-02") == 1); // independent per key
    assert(kc.next("veh-01") == 3);
}

} // namespace

int main() {
    test_uuid_format();
    test_uuid_uniqueness();
    test_uuid_time_ordering();
    test_sequence_counter();
    test_keyed_sequence_counter();
    std::cout << "uuid-sequence-tests: all assertions passed\n";
    return EXIT_SUCCESS;
}
