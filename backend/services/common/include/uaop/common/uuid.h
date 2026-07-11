// uaop::common::uuid_v7 — RFC 9562 UUIDv7 generation. EVENT_FLOW.md §4 requires
// event_id to be a UUIDv7 (time-ordered, so audit/event storage indexes stay
// append-friendly instead of random-insert-friendly like UUIDv4).
//
// @req: UAOP-NFR-008  No exceptions on real-time paths; Result<T,E> pattern platform-wide

#ifndef UAOP_COMMON_UUID_H
#define UAOP_COMMON_UUID_H

#include <array>
#include <chrono>
#include <cstdint>
#include <random>
#include <string>

namespace uaop::common {

namespace detail {
inline std::mt19937_64& uuid_rng() {
    thread_local std::mt19937_64 rng{std::random_device{}()};
    return rng;
}
} // namespace detail

// Layout (RFC 9562 §5.7): 48-bit unix_ts_ms | 4-bit version (0111) | 12-bit rand_a |
// 2-bit variant (10) | 62-bit rand_b.
inline std::string uuid_v7() {
    const auto ms = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count());

    std::uniform_int_distribution<uint64_t> dist;
    const uint64_t rand_a = dist(detail::uuid_rng()) & 0x0FFFULL;         // 12 bits
    const uint64_t rand_b = dist(detail::uuid_rng()) & 0x3FFFFFFFFFFFFFFFULL; // 62 bits

    std::array<uint8_t, 16> b{};
    b[0] = static_cast<uint8_t>((ms >> 40) & 0xFF);
    b[1] = static_cast<uint8_t>((ms >> 32) & 0xFF);
    b[2] = static_cast<uint8_t>((ms >> 24) & 0xFF);
    b[3] = static_cast<uint8_t>((ms >> 16) & 0xFF);
    b[4] = static_cast<uint8_t>((ms >> 8) & 0xFF);
    b[5] = static_cast<uint8_t>(ms & 0xFF);
    b[6] = static_cast<uint8_t>(0x70 | ((rand_a >> 8) & 0x0F)); // version 7
    b[7] = static_cast<uint8_t>(rand_a & 0xFF);
    b[8] = static_cast<uint8_t>(0x80 | ((rand_b >> 56) & 0x3F)); // variant 10
    b[9] = static_cast<uint8_t>((rand_b >> 48) & 0xFF);
    b[10] = static_cast<uint8_t>((rand_b >> 40) & 0xFF);
    b[11] = static_cast<uint8_t>((rand_b >> 32) & 0xFF);
    b[12] = static_cast<uint8_t>((rand_b >> 24) & 0xFF);
    b[13] = static_cast<uint8_t>((rand_b >> 16) & 0xFF);
    b[14] = static_cast<uint8_t>((rand_b >> 8) & 0xFF);
    b[15] = static_cast<uint8_t>(rand_b & 0xFF);

    static constexpr char kHex[] = "0123456789abcdef";
    std::string out;
    out.reserve(36);
    for (std::size_t i = 0; i < b.size(); ++i) {
        if (i == 4 || i == 6 || i == 8 || i == 10) {
            out.push_back('-');
        }
        out.push_back(kHex[(b[i] >> 4) & 0x0F]);
        out.push_back(kHex[b[i] & 0x0F]);
    }
    return out;
}

} // namespace uaop::common

#endif // UAOP_COMMON_UUID_H
