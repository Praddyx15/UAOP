#include <uaop/common/config.h>

#include <cctype>
#include <charconv>
#include <cstdlib>
#include <fstream>
#include <sstream>

namespace uaop::common {

namespace {

std::string trim(const std::string& s) {
    std::size_t start = 0;
    std::size_t end = s.size();
    while (start < end && std::isspace(static_cast<unsigned char>(s[start]))) {
        ++start;
    }
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) {
        --end;
    }
    return s.substr(start, end - start);
}

std::string strip_quotes(const std::string& s) {
    if (s.size() >= 2 && ((s.front() == '"' && s.back() == '"') || (s.front() == '\'' && s.back() == '\''))) {
        return s.substr(1, s.size() - 2);
    }
    return s;
}

} // namespace

Result<std::optional<std::pair<std::string, std::string>>, Error> parse_config_line(const std::string& raw) {
    const std::string line = trim(raw);
    if (line.empty() || line.front() == '#') {
        return Result<std::optional<std::pair<std::string, std::string>>, Error>::Ok(std::nullopt);
    }

    const std::size_t colon = line.find(':');
    if (colon == std::string::npos) {
        return Result<std::optional<std::pair<std::string, std::string>>, Error>::Err(
            Error::Invalid("E_CONFIG_SYNTAX", "expected 'key: value', got: " + line));
    }

    std::string key = trim(line.substr(0, colon));
    std::string value = strip_quotes(trim(line.substr(colon + 1)));
    if (key.empty()) {
        return Result<std::optional<std::pair<std::string, std::string>>, Error>::Err(
            Error::Invalid("E_CONFIG_SYNTAX", "empty key in line: " + line));
    }

    return Result<std::optional<std::pair<std::string, std::string>>, Error>::Ok(
        std::make_pair(std::move(key), std::move(value)));
}

Result<Unit, Error> ConfigStore::load_file(const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) {
        return Result<Unit, Error>::Ok(kUnit); // absent override file is not an error
    }

    std::string line;
    std::size_t lineno = 0;
    while (std::getline(in, line)) {
        ++lineno;
        auto parsed = parse_config_line(line);
        if (!parsed.ok()) {
            return Result<Unit, Error>::Err(Error::Invalid(
                parsed.error().code().data(),
                path + ":" + std::to_string(lineno) + ": " + std::string(parsed.error().message())));
        }
        if (parsed.value().has_value()) {
            values_[parsed.value()->first] = parsed.value()->second;
        }
    }
    return Result<Unit, Error>::Ok(kUnit);
}

void ConfigStore::apply_environment() {
    // POSIX/MinGW: iterate common env vars via getenv is not enumerable
    // portably without platform-specific APIs (environ/_environ). Services
    // pass the key set they care about; we only need to check known keys.
    // For the general case, callers should call apply_environment_for() with
    // the keys their compiled defaults declare — see require()'s companion
    // pattern. This overload covers the documented UAOP_ prefix convention
    // for whatever keys already exist in the store.
    std::vector<std::string> keys;
    keys.reserve(values_.size());
    for (const auto& [key, _] : values_) {
        keys.push_back(key);
    }
    for (const auto& key : keys) {
        std::string env_name = "UAOP_";
        for (char c : key) {
            env_name += (c == '.') ? "__" : std::string(1, static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
        }
        if (const char* v = std::getenv(env_name.c_str())) {
            values_[key] = v;
        }
    }
}

std::optional<std::string> ConfigStore::get(const std::string& key) const {
    auto it = values_.find(key);
    if (it == values_.end()) {
        return std::nullopt;
    }
    return it->second;
}

std::string ConfigStore::get_or(const std::string& key, std::string fallback) const {
    auto v = get(key);
    return v.has_value() ? *v : fallback;
}

std::optional<int64_t> ConfigStore::get_int(const std::string& key) const {
    auto v = get(key);
    if (!v.has_value()) {
        return std::nullopt;
    }
    int64_t out = 0;
    const auto* begin = v->data();
    const auto* end = v->data() + v->size();
    auto [ptr, ec] = std::from_chars(begin, end, out);
    if (ec != std::errc() || ptr != end) {
        return std::nullopt;
    }
    return out;
}

std::optional<double> ConfigStore::get_double(const std::string& key) const {
    auto v = get(key);
    if (!v.has_value()) {
        return std::nullopt;
    }
    double out = 0.0;
    const auto* begin = v->data();
    const auto* end = v->data() + v->size();
    auto [ptr, ec] = std::from_chars(begin, end, out);
    if (ec != std::errc() || ptr != end) {
        return std::nullopt;
    }
    return out;
}

std::optional<bool> ConfigStore::get_bool(const std::string& key) const {
    auto v = get(key);
    if (!v.has_value()) {
        return std::nullopt;
    }
    std::string lower;
    lower.reserve(v->size());
    for (char c : *v) {
        lower += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    if (lower == "true" || lower == "1" || lower == "yes") {
        return true;
    }
    if (lower == "false" || lower == "0" || lower == "no") {
        return false;
    }
    return std::nullopt;
}

Result<Unit, Error> ConfigStore::require(const std::vector<std::string>& keys) const {
    std::string missing;
    for (const auto& key : keys) {
        if (!get(key).has_value()) {
            if (!missing.empty()) {
                missing += ", ";
            }
            missing += key;
        }
    }
    if (!missing.empty()) {
        return Result<Unit, Error>::Err(Error::Fatal("E_CONFIG_MISSING", "missing required config keys: " + missing));
    }
    return Result<Unit, Error>::Ok(kUnit);
}

} // namespace uaop::common
