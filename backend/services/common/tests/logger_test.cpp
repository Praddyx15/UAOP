// @test: UAOP-TC-NFR008-C  Structured JSON logger schema (LOGGING.md §2) and level filtering

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <sstream>

#include <uaop/common/logger.h>

namespace {

using uaop::common::LogLevel;
using uaop::common::Logger;

void test_schema_fields() {
    std::ostringstream out;
    Logger logger("mavlink-bridge", out, LogLevel::kInfo);
    logger.warn("link.rssi_degraded")
        .vehicle("veh-01")
        .corr("01J-test")
        .msg("RSSI below threshold")
        .field("rssi", static_cast<int64_t>(-92))
        .field("link", std::string("serial:/dev/ttyUSB0"));

    const std::string line = out.str();
    assert(line.find("\"lvl\":\"WARN\"") != std::string::npos);
    assert(line.find("\"svc\":\"mavlink-bridge\"") != std::string::npos);
    assert(line.find("\"vehicle\":\"veh-01\"") != std::string::npos);
    assert(line.find("\"corr\":\"01J-test\"") != std::string::npos);
    assert(line.find("\"event\":\"link.rssi_degraded\"") != std::string::npos);
    assert(line.find("\"msg\":\"RSSI below threshold\"") != std::string::npos);
    assert(line.find("\"rssi\":-92") != std::string::npos);
    assert(line.find("\"link\":\"serial:/dev/ttyUSB0\"") != std::string::npos);
    assert(line.back() == '\n');
}

void test_level_filter() {
    std::ostringstream out;
    Logger logger("svc", out, LogLevel::kWarn);
    logger.info("should.be.dropped").msg("never appears");
    assert(out.str().empty());
    logger.error("should.appear").msg("shows up");
    assert(!out.str().empty());
}

void test_json_escaping() {
    std::ostringstream out;
    Logger logger("svc", out, LogLevel::kInfo);
    logger.info("quote.test").msg("has \"quotes\" and \\backslash\\ and\nnewline");
    const std::string line = out.str();
    assert(line.find("\\\"quotes\\\"") != std::string::npos);
    assert(line.find("\\\\backslash\\\\") != std::string::npos);
    assert(line.find("\\n") != std::string::npos);
}

void test_no_double_emit_on_explicit_call() {
    std::ostringstream out;
    Logger logger("svc", out, LogLevel::kInfo);
    {
        auto ev = logger.info("explicit.emit");
        ev.msg("first");
        ev.emit();
    } // destructor must not emit a second line
    std::size_t newline_count = 0;
    for (char c : out.str()) {
        if (c == '\n') {
            ++newline_count;
        }
    }
    assert(newline_count == 1);
}

} // namespace

int main() {
    test_schema_fields();
    test_level_filter();
    test_json_escaping();
    test_no_double_emit_on_explicit_call();
    std::cout << "logger-tests: all assertions passed\n";
    return EXIT_SUCCESS;
}
