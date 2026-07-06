// uaop::common::Error — the boundary error value carried by Result<T, Error>.
// Codes come from the platform registry (api/proto/errors.proto once generated, M1.2);
// until codegen lands, services define their codes against this shape.
//
// @req: UAOP-NFR-008  No exceptions on real-time paths; Result<T,E> pattern platform-wide

#ifndef UAOP_COMMON_ERROR_H
#define UAOP_COMMON_ERROR_H

#include <string>
#include <string_view>
#include <utility>

#include <uaop/common/result.h>

namespace uaop::common {

class Error {
public:
    Error(ErrorClass cls, std::string code, std::string message)
        : class_(cls), code_(std::move(code)), message_(std::move(message)) {}

    [[nodiscard]] ErrorClass error_class() const noexcept { return class_; }

    [[nodiscard]] std::string_view code() const noexcept { return code_; }

    [[nodiscard]] std::string_view message() const noexcept { return message_; }

    [[nodiscard]] bool retryable() const noexcept { return class_ == ErrorClass::kTransient; }

    static Error Transient(std::string code, std::string message) {
        return {ErrorClass::kTransient, std::move(code), std::move(message)};
    }

    static Error Invalid(std::string code, std::string message) {
        return {ErrorClass::kInvalid, std::move(code), std::move(message)};
    }

    static Error Fatal(std::string code, std::string message) {
        return {ErrorClass::kFatal, std::move(code), std::move(message)};
    }

private:
    ErrorClass class_;
    std::string code_;
    std::string message_;
};

} // namespace uaop::common

#endif // UAOP_COMMON_ERROR_H
