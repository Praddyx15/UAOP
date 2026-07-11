#include <uaop/common/logger.h>

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string_view>
#include <type_traits>

namespace uaop::common {

namespace {

void write_json_escaped(std::ostream& out, std::string_view s) {
    out << '"';
    for (char c : s) {
        switch (c) {
            case '"': out << "\\\""; break;
            case '\\': out << "\\\\"; break;
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '\t': out << "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    out << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                        << static_cast<int>(c) << std::dec;
                } else {
                    out << c;
                }
        }
    }
    out << '"';
}

// RFC 3339 UTC with millisecond precision: "2026-07-03T10:15:04.221Z" (LOGGING.md §2).
std::string rfc3339_now() {
    const auto now = std::chrono::system_clock::now();
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    const std::time_t t = std::chrono::system_clock::to_time_t(now);

    std::tm tm_utc{};
#if defined(_WIN32)
    gmtime_s(&tm_utc, &t);
#else
    gmtime_r(&t, &tm_utc);
#endif

    std::ostringstream oss;
    oss << std::put_time(&tm_utc, "%Y-%m-%dT%H:%M:%S");
    oss << '.' << std::setw(3) << std::setfill('0') << ms.count() << 'Z';
    return oss.str();
}

} // namespace

LogEvent::LogEvent(Logger& owner, LogLevel level, std::string event)
    : owner_(&owner), level_(level), event_(std::move(event)) {}

LogEvent::LogEvent(LogEvent&& other) noexcept
    : owner_(other.owner_),
      level_(other.level_),
      event_(std::move(other.event_)),
      msg_(std::move(other.msg_)),
      vehicle_(std::move(other.vehicle_)),
      corr_(std::move(other.corr_)),
      fields_(std::move(other.fields_)),
      emitted_(other.emitted_) {
    other.emitted_ = true; // moved-from: destructor must not double-emit
}

LogEvent::~LogEvent() {
    if (!emitted_) {
        emit();
    }
}

void LogEvent::emit() {
    if (emitted_) {
        return;
    }
    emitted_ = true;

    if (owner_ == nullptr || level_ < owner_->min_level()) {
        return; // below the runtime level toggle — never even formatted
    }

    std::ostream& out = owner_->stream();
    out << "{\"ts\":";
    write_json_escaped(out, rfc3339_now());
    out << ",\"lvl\":\"" << to_string(level_) << '"';
    out << ",\"svc\":";
    write_json_escaped(out, owner_->service());
    if (!vehicle_.empty()) {
        out << ",\"vehicle\":";
        write_json_escaped(out, vehicle_);
    }
    if (!corr_.empty()) {
        out << ",\"corr\":";
        write_json_escaped(out, corr_);
    }
    out << ",\"event\":";
    write_json_escaped(out, event_);
    if (!msg_.empty()) {
        out << ",\"msg\":";
        write_json_escaped(out, msg_);
    }
    for (const auto& [key, value] : fields_) {
        out << ',';
        write_json_escaped(out, key);
        out << ':';
        std::visit(
            [&out](const auto& v) {
                using T = std::decay_t<decltype(v)>;
                if constexpr (std::is_same_v<T, std::string>) {
                    write_json_escaped(out, v);
                } else if constexpr (std::is_same_v<T, bool>) {
                    out << (v ? "true" : "false");
                } else {
                    out << v;
                }
            },
            value);
    }
    out << "}\n";
}

} // namespace uaop::common
