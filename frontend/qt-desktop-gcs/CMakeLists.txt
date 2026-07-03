cmake_minimum_required(VERSION 3.16)

project(UAOPGCS LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configure Qt6
find_package(Qt6 COMPONENTS Gui Qml Quick REQUIRED)

qt_standard_project_setup()

# Set up project sources
set(PROJECT_SOURCES
    src/main.cpp
    src/TelemetryController.h
    src/TelemetryController.cpp
)

# Add the QML application target
qt_add_executable(uaop_gcs ${PROJECT_SOURCES})

# Define the QML module and add QML source files
qt_add_qml_module(uaop_gcs
    URI UAOPGCS
    VERSION 1.0
    QML_FILES
        ui/main.qml
        ui/styles/Theme.qml
        ui/components/ComplianceCheckItem.qml
        ui/components/NavigationButton.qml
        ui/views/telemetry/TelemetryView.qml
        ui/views/compliance/ComplianceView.qml
        ui/views/defence/DefenceView.qml
        ui/views/mission/MissionView.qml
)

# Target dependencies
target_link_libraries(uaop_gcs PRIVATE
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
)

# Compiler warning flags
if(MSVC)
    target_compile_options(uaop_gcs PRIVATE /W4 /WX)
else()
    target_compile_options(uaop_gcs PRIVATE -Wall -Wextra -Werror)
endif()
