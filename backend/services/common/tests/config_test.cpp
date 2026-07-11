// @test: UAOP-TC-NFR008-D  Layered config: defaults -> file -> env, parse + require()

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>

#include <uaop/common/config.h>

namespace {

using uaop::common::ConfigStore;
using uaop::common::parse_config_line;

void test_parse_config_line() {
    auto blank = parse_config_line("   ");
    assert(blank.ok() && !blank.value().has_value());

    auto comment = parse_config_line("# a comment");
    assert(comment.ok() && !comment.value().has_value());

    auto kv = parse_config_line("nats.url: nats://localhost:4222");
    assert(kv.ok() && kv.value().has_value());
    assert(kv.value()->first == "nats.url");
    assert(kv.value()->second == "nats://localhost:4222");

    auto quoted = parse_config_line("name: \"quoted value\"");
    assert(quoted.ok() && quoted.value()->second == "quoted value");

    auto malformed = parse_config_line("no colon here");
    assert(!malformed.ok());
}

void test_layering_precedence() {
    ConfigStore store({{"log.level", "info"}, {"nats.url", "nats://default:4222"}});
    assert(store.get_or("log.level", "") == "info");

    const std::string path = "config_test_override.yaml.tmp";
    {
        std::ofstream f(path);
        f << "nats.url: nats://file-override:4222\n";
        f << "new.key: from-file\n";
    }
    auto loaded = store.load_file(path);
    assert(loaded.ok());
    assert(store.get_or("nats.url", "") == "nats://file-override:4222"); // file overrides default
    assert(store.get_or("new.key", "") == "from-file");
    assert(store.get_or("log.level", "") == "info"); // untouched key survives
    std::remove(path.c_str());

    auto missing_file = store.load_file("does_not_exist.yaml.tmp");
    assert(missing_file.ok()); // absent override file is not an error
}

void test_env_override() {
    ConfigStore store(std::unordered_map<std::string, std::string>{{"nats.url", "nats://default:4222"}});
#if defined(_WIN32)
    _putenv_s("UAOP_NATS__URL", "nats://env-override:4222");
#else
    setenv("UAOP_NATS__URL", "nats://env-override:4222", 1);
#endif
    store.apply_environment();
    assert(store.get_or("nats.url", "") == "nats://env-override:4222");
}

void test_typed_getters() {
    ConfigStore store({{"port", "50051"}, {"ratio", "0.5"}, {"enabled", "true"}, {"bad", "not-a-number"}});
    assert(store.get_int("port").value() == 50051);
    assert(store.get_double("ratio").value() == 0.5);
    assert(store.get_bool("enabled").value() == true);
    assert(!store.get_int("bad").has_value());
    assert(!store.get_int("nonexistent").has_value());
}

void test_require() {
    ConfigStore store(std::unordered_map<std::string, std::string>{{"a", "1"}});
    assert(store.require({"a"}).ok());
    auto missing = store.require({"a", "b", "c"});
    assert(!missing.ok());
    assert(missing.error().error_class() == uaop::common::ErrorClass::kFatal);
}

} // namespace

int main() {
    test_parse_config_line();
    test_layering_precedence();
    test_env_override();
    test_typed_getters();
    test_require();
    std::cout << "config-tests: all assertions passed\n";
    return EXIT_SUCCESS;
}
