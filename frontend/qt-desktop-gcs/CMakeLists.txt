cmake_minimum_required(VERSION 3.25)

project(UAOPGCS LANGUAGES CXX)

# GCS is C++20 (ADR-0016); services stay C++17.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Gui Qml Quick QuickControls2 REQUIRED)

qt_standard_project_setup()

qt_add_executable(uaop_gcs
    src/main.cpp
    src/TelemetryController.h
    src/TelemetryController.cpp
    src/core/PanelRegistry.h
    src/core/PanelRegistry.cpp
    src/core/WorkspaceManager.h
    src/core/WorkspaceManager.cpp
)

target_include_directories(uaop_gcs PRIVATE src)

qt_add_qml_module(uaop_gcs
    URI UAOPGCS
    VERSION 1.0
    QML_FILES
        ui/Shell.qml
        ui/styles/Theme.qml
        ui/components/PanelChrome.qml
        ui/components/PanelHost.qml
        ui/components/ComplianceCheckItem.qml
        ui/views/telemetry/TelemetryView.qml
        ui/views/compliance/ComplianceView.qml
        ui/views/defence/DefenceView.qml
        ui/views/mission/MissionView.qml
)

target_link_libraries(uaop_gcs PRIVATE
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
)

if(MSVC)
    target_compile_options(uaop_gcs PRIVATE /W4 /WX)
else()
    target_compile_options(uaop_gcs PRIVATE -Wall -Wextra -Werror)
endif()

# Headless smoke gate: shell must load with no vehicle connected (M0 DoD).
if(BUILD_TESTING)
    include(CTest)
    add_test(NAME gcs.selfcheck COMMAND uaop_gcs --selfcheck)
    set_tests_properties(gcs.selfcheck PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
        TIMEOUT 60)
endif()
