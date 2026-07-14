cmake_minimum_required(VERSION 3.25)

project(uaop-platform
    VERSION 0.1.0
    DESCRIPTION "UAOP - Unified Autonomy Operating Platform"
    LANGUAGES CXX)

option(UAOP_BUILD_SERVICES "Build C++ services and common library" ON)
option(UAOP_BUILD_GCS "Build the Qt desktop GCS (requires Qt 6 on CMAKE_PREFIX_PATH)" OFF)
option(UAOP_BUILD_TEMPLATE "Build the canonical service template (M0 proof target)" ON)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

# Warnings-as-errors platform-wide (CODING_STANDARDS.md §1)
add_library(uaop_warnings INTERFACE)
if(MSVC)
    target_compile_options(uaop_warnings INTERFACE /W4 /WX)
else()
    target_compile_options(uaop_warnings INTERFACE -Wall -Wextra -Wconversion -Werror)
endif()

include(CTest)

# uaop::proto (ADR-0021) — protobuf codegen for platform contracts. Local
# Windows/MinGW dev may not have protobuf available; CI (apt-installed)
# always does. Anything needing uaop::proto checks UAOP_PROTO_AVAILABLE.
add_subdirectory(api/proto)
if(NOT UAOP_PROTO_AVAILABLE)
    set(UAOP_PROTO_AVAILABLE FALSE)
endif()

if(UAOP_BUILD_SERVICES)
    add_subdirectory(backend/services/common)
endif()

if(UAOP_BUILD_TEMPLATE)
    add_subdirectory(tools/service-template)
endif()

# middleware/mavlink-bridge (M2.1) links uaop::proto once its own
# CMakeLists.txt lands — not yet, so nothing to add_subdirectory here.

if(UAOP_BUILD_GCS)
    add_subdirectory(frontend/qt-desktop-gcs)
endif()
