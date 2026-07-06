// uaop::common::Result<T, E> — error handling without exceptions on flight-influencing
// paths (ADR-0013; CODING_STANDARDS.md §1). Errors are values; every Result must be
// consumed ([[nodiscard]]).
//
// @req: UAOP-NFR-008  No exceptions on real-time paths; Result<T,E> pattern platform-wide

#ifndef UAOP_COMMON_RESULT_H
#define UAOP_COMMON_RESULT_H

#include <type_traits>
#include <utility>
#include <variant>

namespace uaop::common {

// Error taxonomy at service boundaries (SOFTWARE_ARCHITECTURE.md §5):
// TRANSIENT -> retry with backoff · INVALID -> reject to caller · FATAL -> supervised restart
enum class ErrorClass : int { kTransient = 0, kInvalid = 1, kFatal = 2 };

template <typename T, typename E>
class [[nodiscard]] Result {
    static_assert(!std::is_same_v<T, E>, "Result<T,E> requires distinct value and error types");

public:
    static Result Ok(T value) { return Result(std::in_place_index<0>, std::move(value)); }

    static Result Err(E error) { return Result(std::in_place_index<1>, std::move(error)); }

    [[nodiscard]] bool ok() const noexcept { return data_.index() == 0; }

    // Precondition: ok(). Enforced by convention + review; std::get terminates on misuse
    // in no-exception builds rather than corrupting state.
    [[nodiscard]] const T& value() const& noexcept { return std::get<0>(data_); }

    [[nodiscard]] T& value() & noexcept { return std::get<0>(data_); }

    [[nodiscard]] T&& value() && noexcept { return std::get<0>(std::move(data_)); }

    // Precondition: !ok().
    [[nodiscard]] const E& error() const& noexcept { return std::get<1>(data_); }

    [[nodiscard]] E& error() & noexcept { return std::get<1>(data_); }

    [[nodiscard]] T value_or(T fallback) const& {
        return ok() ? std::get<0>(data_) : std::move(fallback);
    }

    // Monadic composition: fn is applied to the value; errors pass through unchanged.
    template <typename Fn>
    [[nodiscard]] auto map(Fn&& fn) const& -> Result<std::invoke_result_t<Fn, const T&>, E> {
        using U = std::invoke_result_t<Fn, const T&>;
        if (ok()) {
            return Result<U, E>::Ok(std::forward<Fn>(fn)(std::get<0>(data_)));
        }
        return Result<U, E>::Err(std::get<1>(data_));
    }

    // and_then: fn returns a Result itself; short-circuits on error.
    template <typename Fn>
    [[nodiscard]] auto and_then(Fn&& fn) const& -> std::invoke_result_t<Fn, const T&> {
        using R = std::invoke_result_t<Fn, const T&>;
        if (ok()) {
            return std::forward<Fn>(fn)(std::get<0>(data_));
        }
        return R::Err(std::get<1>(data_));
    }

private:
    template <std::size_t I, typename V>
    Result(std::in_place_index_t<I> tag, V&& v) : data_(tag, std::forward<V>(v)) {}

    std::variant<T, E> data_;
};

// Unit type for Result<Unit, E> when an operation returns no value.
struct Unit {};

inline constexpr Unit kUnit{};

} // namespace uaop::common

#endif // UAOP_COMMON_RESULT_H
