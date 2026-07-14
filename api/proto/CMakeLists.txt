# C++ protobuf codegen for the platform-wide proto contracts (ADR-0021).
# Every non-GCS C++ service links uaop::proto instead of generating its own
# stubs — one codegen target, one place the build's protobuf toolchain
# requirement lives.
#
# Provisioning is intentionally asymmetric and documented as such (ADR-0021):
# CI (Ubuntu) installs protobuf-compiler/libprotobuf-dev via apt before this
# runs, so find_package succeeds there. Local Windows/MinGW dev has no
# equivalent prebuilt package for this exact toolchain; rather than force a
# slow, failure-prone FetchContent source build into every contributor's
# path, this target degrades to "skipped, with a clear message" when
# protobuf isn't found, and anything that depends on it (mavlink-bridge)
# does the same. Real verification happens on CI, where the dependency is
# actually present.

find_package(Protobuf CONFIG QUIET)
if(NOT Protobuf_FOUND)
    find_package(Protobuf QUIET)
endif()

if(NOT Protobuf_FOUND)
    message(STATUS "api/proto: Protobuf not found -- uaop::proto and anything depending on it will be skipped locally (CI installs it via apt; see ADR-0021)")
    set(UAOP_PROTO_AVAILABLE FALSE PARENT_SCOPE)
    return()
endif()

set(UAOP_PROTO_AVAILABLE TRUE PARENT_SCOPE)
message(STATUS "api/proto: Protobuf ${Protobuf_VERSION} found -- building uaop::proto")

set(UAOP_PROTO_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(UAOP_PROTO_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/gen)
file(MAKE_DIRECTORY ${UAOP_PROTO_GEN_DIR})

set(UAOP_PROTO_FILES
    uaop/telemetry/v1/vehicle_telemetry.proto
    uaop/telemetry/v1/snapshot.proto
    uaop/telemetry/v1/delta.proto
    uaop/events/v1/envelope.proto
    uaop/errors/v1/errors.proto
)

set(UAOP_PROTO_SOURCES "")
set(UAOP_PROTO_HEADERS "")
foreach(proto_file ${UAOP_PROTO_FILES})
    get_filename_component(proto_dir ${proto_file} DIRECTORY)
    get_filename_component(proto_name ${proto_file} NAME_WE)
    set(gen_cc ${UAOP_PROTO_GEN_DIR}/${proto_dir}/${proto_name}.pb.cc)
    set(gen_h ${UAOP_PROTO_GEN_DIR}/${proto_dir}/${proto_name}.pb.h)
    add_custom_command(
        OUTPUT ${gen_cc} ${gen_h}
        COMMAND protobuf::protoc
        ARGS --cpp_out=${UAOP_PROTO_GEN_DIR} -I ${UAOP_PROTO_ROOT} ${UAOP_PROTO_ROOT}/${proto_file}
        DEPENDS ${UAOP_PROTO_ROOT}/${proto_file} protobuf::protoc
        COMMENT "Generating C++ protobuf sources for ${proto_file}"
        VERBATIM
    )
    list(APPEND UAOP_PROTO_SOURCES ${gen_cc})
    list(APPEND UAOP_PROTO_HEADERS ${gen_h})
endforeach()

add_library(uaop-proto STATIC ${UAOP_PROTO_SOURCES})
add_library(uaop::proto ALIAS uaop-proto)
target_include_directories(uaop-proto PUBLIC $<BUILD_INTERFACE:${UAOP_PROTO_GEN_DIR}>)
target_link_libraries(uaop-proto PUBLIC protobuf::libprotobuf)
target_compile_features(uaop-proto PUBLIC cxx_std_17)
# Generated code isn't ours to hold to the platform's own warning bar.
if(NOT MSVC)
    target_compile_options(uaop-proto PRIVATE -Wno-unused-parameter)
endif()
