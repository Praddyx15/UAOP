add_executable(uaop-common-tests result_test.cpp)
target_link_libraries(uaop-common-tests PRIVATE uaop::common uaop_warnings)
target_compile_features(uaop-common-tests PRIVATE cxx_std_17)

add_test(NAME common.result COMMAND uaop-common-tests)
