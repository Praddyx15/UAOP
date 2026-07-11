add_executable(uaop-common-tests result_test.cpp)
target_link_libraries(uaop-common-tests PRIVATE uaop::common uaop_warnings)
target_compile_features(uaop-common-tests PRIVATE cxx_std_17)
add_test(NAME common.result COMMAND uaop-common-tests)

add_executable(uaop-common-uuid-sequence-tests uuid_sequence_test.cpp)
target_link_libraries(uaop-common-uuid-sequence-tests PRIVATE uaop::common uaop_warnings)
target_compile_features(uaop-common-uuid-sequence-tests PRIVATE cxx_std_17)
add_test(NAME common.uuid_sequence COMMAND uaop-common-uuid-sequence-tests)

add_executable(uaop-common-logger-tests logger_test.cpp)
target_link_libraries(uaop-common-logger-tests PRIVATE uaop::common uaop_warnings)
target_compile_features(uaop-common-logger-tests PRIVATE cxx_std_17)
add_test(NAME common.logger COMMAND uaop-common-logger-tests)

add_executable(uaop-common-config-tests config_test.cpp)
target_link_libraries(uaop-common-config-tests PRIVATE uaop::common uaop_warnings)
target_compile_features(uaop-common-config-tests PRIVATE cxx_std_17)
add_test(NAME common.config COMMAND uaop-common-config-tests)

add_executable(uaop-common-metrics-health-tests metrics_health_test.cpp)
target_link_libraries(uaop-common-metrics-health-tests PRIVATE uaop::common uaop_warnings)
target_compile_features(uaop-common-metrics-health-tests PRIVATE cxx_std_17)
add_test(NAME common.metrics_health COMMAND uaop-common-metrics-health-tests)

add_executable(uaop-common-dedup-jetstream-tests dedup_jetstream_test.cpp)
target_link_libraries(uaop-common-dedup-jetstream-tests PRIVATE uaop::common uaop_warnings)
target_compile_features(uaop-common-dedup-jetstream-tests PRIVATE cxx_std_17)
add_test(NAME common.dedup_jetstream COMMAND uaop-common-dedup-jetstream-tests)

add_executable(uaop-common-health-metrics-server-tests health_metrics_server_test.cpp)
target_link_libraries(uaop-common-health-metrics-server-tests PRIVATE uaop::common uaop_warnings)
target_compile_features(uaop-common-health-metrics-server-tests PRIVATE cxx_std_17)
add_test(NAME common.health_metrics_server COMMAND uaop-common-health-metrics-server-tests)
