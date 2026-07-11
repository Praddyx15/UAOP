// uaop::common::ConfigStore — the layered config system (SOFTWARE_ARCHITECTURE.md
// §6), minus layer 5 (the Postgres-backed runtime settings API, which is
// necessarily per-service since it needs that service's DB role). Layers,
// lowest to highest precedence:
//   1. Compiled defaults      — passed in by the caller
//   2. /etc/uaop/platform.yaml — deployment-profile file (optional)
//   3. Per-service override file (same schema subtree, optional)
//   4. Environment variables  — UAOP_FOO__BAR overrides key "foo.bar"
// A service that cannot validate its config fails fast (SOFTWARE_ARCHITECTURE.md
// §6) — require() returns an Error rather than the caller discovering a blank
// value three layers deep in some other function.
//
// File format: a deliberately restricted YAML-compatible subset — flat or
// dot-path `key: value` lines, `#` comments, no lists/anchors/multi-doc. Every
// file written against this subset parses correctly under a full YAML parser
// too, so adopting one later (if a config ever needs real YAML features) is a
// parser swap, not a file-format migration.
//
// @req: UAOP-NFR-008  No exceptions on real-time paths; Result<T,E> pattern platform-wide

#ifndef UAOP_COMMON_CONFIG_H
#define UAOP_COMMON_CONFIG_H

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <uaop/common/error.h>
#include <uaop/common/result.h>

namespace uaop::common {

class ConfigStore {
public:
    explicit ConfigStore(std::unordered_map<std::string, std::string> compiled_defaults = {})
        : values_(std::move(compiled_defaults)) {}

    // Layers 2/3: merges (overwrites on key collision) key:value pairs parsed
    // from `path`. A missing file is not an error — most services have no
    // per-service override file — but a malformed one is.
    [[nodiscard]] Result<Unit, Error> load_file(const std::string& path);

    // Layer 4: scans the process environment for UAOP_<KEY> (double underscore
    // = dot separator, e.g. UAOP_NATS__URL -> "nats.url"), applied over
    // whatever load_file calls already ran.
    void apply_environment();

    [[nodiscard]] std::optional<std::string> get(const std::string& key) const;
    [[nodiscard]] std::string get_or(const std::string& key, std::string fallback) const;
    [[nodiscard]] std::optional<int64_t> get_int(const std::string& key) const;
    [[nodiscard]] std::optional<double> get_double(const std::string& key) const;
    [[nodiscard]] std::optional<bool> get_bool(const std::string& key) const;

    void set(std::string key, std::string value) { values_[std::move(key)] = std::move(value); }

    // Fails fast (SOFTWARE_ARCHITECTURE.md §6): returns every missing key at
    // once rather than one exception per missing key, so a misconfigured
    // service reports its whole problem on the first failed startup, not one
    // fix-rebuild-fail cycle at a time.
    [[nodiscard]] Result<Unit, Error> require(const std::vector<std::string>& keys) const;

private:
    std::unordered_map<std::string, std::string> values_;
};

// Parses one restricted-YAML `key: value` line. Exposed for unit testing;
// returns std::nullopt for blank/comment lines, Error for malformed content.
[[nodiscard]] Result<std::optional<std::pair<std::string, std::string>>, Error>
parse_config_line(const std::string& line);

} // namespace uaop::common

#endif // UAOP_COMMON_CONFIG_H
