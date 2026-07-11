// uaop::common::Logger — structured JSON diagnostic logging (LOGGING.md §2).
// This is stream 3 of 3 (diagnostic logs) — never a substitute for the audit
// stream or the telemetry store (LOGGING.md §1). One schema platform-wide:
// {"ts":...,"lvl":...,"svc":...,"vehicle":...,"corr":...,"event":...,"msg":...,
//  ...extra fields}. Variables go in fields, never interpolated into `msg`
// (LOGGING.md §2) — that's what makes this greppable/alertable.
//
// Hot-path discipline (LOGGING.md §2, ADR-0013): never call this inside the RT
// telemetry path except periodic flushed counters — that's a call-site
// discipline this header cannot enforce, only document.

#ifndef UAOP_COMMON_LOGGER_H
#define UAOP_COMMON_LOGGER_H

#include <atomic>
#include <cstdint>
#include <ostream>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace uaop::common {

enum class LogLevel : int { kDebug = 0, kInfo = 1, kWarn = 2, kError = 3 };

inline const char* to_string(LogLevel lvl) noexcept {
    switch (lvl) {
        case LogLevel::kDebug: return "DEBUG";
        case LogLevel::kInfo: return "INFO";
        case LogLevel::kWarn: return "WARN";
        case LogLevel::kError: return "ERROR";
    }
    return "UNKNOWN";
}

class Logger;

// One log line under construction. Built via Logger::debug/info/warn/error(event),
// fields chained on, emitted to the logger's stream when the object is
// destroyed (or early via emit()). event is a stable snake_case identifier;
// msg is prose for humans.
class LogEvent {
public:
    LogEvent(Logger& owner, LogLevel level, std::string event);
    LogEvent(const LogEvent&) = delete;
    LogEvent& operator=(const LogEvent&) = delete;
    LogEvent(LogEvent&& other) noexcept;
    LogEvent& operator=(LogEvent&&) = delete;
    ~LogEvent();

    LogEvent& msg(std::string text) {
        msg_ = std::move(text);
        return *this;
    }

    LogEvent& vehicle(std::string vehicle_id) {
        vehicle_ = std::move(vehicle_id);
        return *this;
    }

    LogEvent& corr(std::string correlation_id) {
        corr_ = std::move(correlation_id);
        return *this;
    }

    LogEvent& field(std::string key, std::string value) {
        fields_.emplace_back(std::move(key), Value{std::move(value)});
        return *this;
    }

    LogEvent& field(std::string key, int64_t value) {
        fields_.emplace_back(std::move(key), Value{value});
        return *this;
    }

    LogEvent& field(std::string key, double value) {
        fields_.emplace_back(std::move(key), Value{value});
        return *this;
    }

    LogEvent& field(std::string key, bool value) {
        fields_.emplace_back(std::move(key), Value{value});
        return *this;
    }

    // Emits immediately; safe to call at most once, and safe to skip (the
    // destructor emits if this wasn't called — never a silent drop).
    void emit();

private:
    using Value = std::variant<std::string, int64_t, double, bool>;

    Logger* owner_;
    LogLevel level_;
    std::string event_;
    std::string msg_;
    std::string vehicle_;
    std::string corr_;
    std::vector<std::pair<std::string, Value>> fields_;
    bool emitted_ = false;

    friend class Logger;
};

// One Logger per process/service (constructed once with the svc name).
// min_level is runtime-toggleable (LOGGING.md §2: "audit-logged toggle" —
// the audit-logging of the toggle itself is the caller's responsibility, e.g.
// emit an audit event when set_min_level changes production DEBUG state).
class Logger {
public:
    explicit Logger(std::string service, std::ostream& out, LogLevel min_level = LogLevel::kInfo)
        : service_(std::move(service)), out_(out), min_level_(min_level) {}

    [[nodiscard]] LogEvent debug(std::string event) { return LogEvent(*this, LogLevel::kDebug, std::move(event)); }
    [[nodiscard]] LogEvent info(std::string event) { return LogEvent(*this, LogLevel::kInfo, std::move(event)); }
    [[nodiscard]] LogEvent warn(std::string event) { return LogEvent(*this, LogLevel::kWarn, std::move(event)); }
    [[nodiscard]] LogEvent error(std::string event) { return LogEvent(*this, LogLevel::kError, std::move(event)); }

    void set_min_level(LogLevel lvl) noexcept { min_level_.store(lvl, std::memory_order_relaxed); }

    [[nodiscard]] LogLevel min_level() const noexcept { return min_level_.load(std::memory_order_relaxed); }

    [[nodiscard]] const std::string& service() const noexcept { return service_; }

    [[nodiscard]] std::ostream& stream() noexcept { return out_; }

private:
    std::string service_;
    std::ostream& out_;
    std::atomic<LogLevel> min_level_;
};

} // namespace uaop::common

#endif // UAOP_COMMON_LOGGER_H
