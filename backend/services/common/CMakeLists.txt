add_library(uaop-common STATIC
    src/config.cpp
    src/health.cpp
    src/health_metrics_server.cpp
    src/in_memory_jetstream.cpp
    src/logger.cpp
    src/metrics.cpp)
add_library(uaop::common ALIAS uaop-common)

target_include_directories(uaop-common PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

target_compile_features(uaop-common PUBLIC cxx_std_17)

if(WIN32)
    target_link_libraries(uaop-common PUBLIC ws2_32)
else()
    find_package(Threads REQUIRED)
    target_link_libraries(uaop-common PUBLIC Threads::Threads)
endif()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
