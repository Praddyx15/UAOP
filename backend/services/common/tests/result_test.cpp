// Unit tests for uaop::common::Result / Error.
// Framework note: plain assert-based until the test framework lands with M1 (TESTING.md);
// wired into ctest so the gate exists from the first commit.
//
// @test: UAOP-TC-NFR008-A  Result<T,E> value/error paths, taxonomy, monadic composition

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string>

#include <uaop/common/error.h>
#include <uaop/common/result.h>

namespace {

using uaop::common::Error;
using uaop::common::ErrorClass;
using uaop::common::Result;
using uaop::common::Unit;

Result<int, Error> parse_positive(int raw) {
    if (raw <= 0) {
        return Result<int, Error>::Err(Error::Invalid("E_RANGE", "must be positive"));
    }
    return Result<int, Error>::Ok(raw);
}

void test_ok_path() {
    const auto r = parse_positive(42);
    assert(r.ok());
    assert(r.value() == 42);
    assert(r.value_or(-1) == 42);
}

void test_err_path() {
    const auto r = parse_positive(-5);
    assert(!r.ok());
    assert(r.error().code() == "E_RANGE");
    assert(r.error().error_class() == ErrorClass::kInvalid);
    assert(!r.error().retryable());
    assert(r.value_or(-1) == -1);
}

void test_taxonomy() {
    assert(Error::Transient("E_T", "t").retryable());
    assert(!Error::Invalid("E_I", "i").retryable());
    assert(!Error::Fatal("E_F", "f").retryable());
    assert(Error::Fatal("E_F", "f").error_class() == ErrorClass::kFatal);
}

void test_map_and_then() {
    const auto doubled = parse_positive(21).map([](const int& v) { return v * 2; });
    assert(doubled.ok());
    assert(doubled.value() == 42);

    const auto chained = parse_positive(10).and_then(
        [](const int& v) { return parse_positive(v - 20); }); // second step fails
    assert(!chained.ok());
    assert(chained.error().code() == "E_RANGE");

    const auto short_circuit =
        parse_positive(-1).map([](const int& v) { return v * 1000; }); // never runs
    assert(!short_circuit.ok());
}

void test_unit_result() {
    const auto r = Result<Unit, Error>::Ok(uaop::common::kUnit);
    assert(r.ok());
}

} // namespace

int main() {
    test_ok_path();
    test_err_path();
    test_taxonomy();
    test_map_and_then();
    test_unit_result();
    std::cout << "uaop-common-tests: all assertions passed\n";
    return EXIT_SUCCESS;
}
