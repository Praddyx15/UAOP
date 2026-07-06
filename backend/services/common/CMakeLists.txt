add_library(uaop-common INTERFACE)
add_library(uaop::common ALIAS uaop-common)

target_include_directories(uaop-common INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

target_compile_features(uaop-common INTERFACE cxx_std_17)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
