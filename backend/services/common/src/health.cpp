#include <uaop/common/health.h>

#include <sstream>

namespace uaop::common {

namespace {
void write_json_escaped(std::ostream& out, const std::string& s) {
    out << '"';
    for (char c : s) {
        if (c == '"' || c == '\\') {
            out << '\\';
        }
        out << c;
    }
    out << '"';
}
} // namespace

void HealthRegistry::set(const std::string& check_name, HealthState state, std::string reason) {
    std::lock_guard<std::mutex> lock(mutex_);
    checks_[check_name] = Check{state, std::move(reason)};
}

HealthState HealthRegistry::overall() const {
    std::lock_guard<std::mutex> lock(mutex_);
    HealthState worst = HealthState::kOk;
    for (const auto& [name, check] : checks_) {
        if (static_cast<int>(check.state) > static_cast<int>(worst)) {
            worst = check.state;
        }
    }
    return worst;
}

std::string HealthRegistry::render_json() const {
    std::lock_guard<std::mutex> lock(mutex_);
    HealthState worst = HealthState::kOk;
    std::ostringstream checks_json;
    checks_json << '{';
    bool first = true;
    for (const auto& [name, check] : checks_) {
        if (static_cast<int>(check.state) > static_cast<int>(worst)) {
            worst = check.state;
        }
        if (!first) {
            checks_json << ',';
        }
        first = false;
        write_json_escaped(checks_json, name);
        checks_json << ":{\"state\":\"" << to_string(check.state) << "\",\"reason\":";
        write_json_escaped(checks_json, check.reason);
        checks_json << '}';
    }
    checks_json << '}';

    std::ostringstream oss;
    oss << "{\"status\":\"" << to_string(worst) << "\",\"checks\":" << checks_json.str() << '}';
    return oss.str();
}

} // namespace uaop::common
