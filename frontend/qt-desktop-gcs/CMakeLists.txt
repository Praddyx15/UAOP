cmake_minimum_required(VERSION 3.25)

project(UAOPGCS LANGUAGES CXX)

# GCS is C++20 (ADR-0016); services stay C++17.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Gui Qml Quick QuickControls2 Sql Network REQUIRED)

qt_standard_project_setup()

qt_add_executable(uaop_gcs
    src/main.cpp
    src/TelemetryController.h
    src/TelemetryController.cpp
    src/core/PanelRegistry.h
    src/core/PanelRegistry.cpp
    src/core/WorkspaceManager.h
    src/core/WorkspaceManager.cpp
    src/core/Suite.h
    src/map/SlippyMath.h
    src/map/MbtilesProvider.h
    src/map/MbtilesProvider.cpp
    src/models/FlightReportJson.h
    src/models/FlightReportJson.cpp
    src/models/FlightReportsModel.h
    src/models/FlightReportsModel.cpp
)

target_include_directories(uaop_gcs PRIVATE src)

# Geist / Geist Mono (OFL 1.1, resources/fonts/Geist/OFL.txt) compiled in as resources,
# so nothing depends on what fonts the host has (STYLE_GUIDE.md §3).
qt_add_resources(uaop_gcs "fonts"
    PREFIX "/fonts"
    BASE "resources/fonts/Geist"
    FILES
        resources/fonts/Geist/Geist-Light.ttf
        resources/fonts/Geist/Geist-LightItalic.ttf
        resources/fonts/Geist/Geist-Regular.ttf
        resources/fonts/Geist/Geist-Italic.ttf
        resources/fonts/Geist/Geist-Medium.ttf
        resources/fonts/Geist/Geist-MediumItalic.ttf
        resources/fonts/Geist/Geist-Bold.ttf
        resources/fonts/Geist/Geist-BoldItalic.ttf
        resources/fonts/Geist/GeistMono-Regular.ttf
        resources/fonts/Geist/GeistMono-Medium.ttf
        resources/fonts/Geist/GeistMono-Bold.ttf
)

# Interim MBTiles renderer points at the small synthetic test tile set
# (tests/data/map/world.mbtiles, ~85 tiles, ADR-0020) until DEPLOYMENT.md's
# MapConfig wires a real operator-supplied basemap path.
target_compile_definitions(uaop_gcs PRIVATE
    UAOP_MBTILES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../../tests/data/map/world.mbtiles")

# flight-log-engine's analysis sidecar on the node (ADR-0022); UAOP_FLIGHT_LOG_URL overrides it at run time.
target_compile_definitions(uaop_gcs PRIVATE UAOP_FLIGHT_LOG_URL_DEFAULT="http://127.0.0.1:8095")

qt_add_qml_module(uaop_gcs
    URI UAOPGCS
    VERSION 1.0
    QML_FILES
        ui/Shell.qml
        ui/styles/Theme.qml
        ui/components/PanelChrome.qml
        ui/components/PanelHost.qml
        ui/components/ComplianceCheckItem.qml
        ui/components/MapView.qml
        ui/components/StatusTag.qml
        ui/views/telemetry/TelemetryView.qml
        ui/views/compliance/ComplianceView.qml
        ui/views/defence/DefenceView.qml
        ui/views/mission/MissionView.qml
        ui/views/logs/PostMissionView.qml
)

target_link_libraries(uaop_gcs PRIVATE
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Sql
    Qt6::Network
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

    add_executable(uaop-gcs-slippy-math-test tests/slippy_math_test.cpp)
    target_include_directories(uaop-gcs-slippy-math-test PRIVATE src)
    target_compile_features(uaop-gcs-slippy-math-test PRIVATE cxx_std_20)
    add_test(NAME gcs.slippy_math COMMAND uaop-gcs-slippy-math-test)

    add_executable(uaop-gcs-mbtiles-test tests/mbtiles_provider_test.cpp
                                          src/map/MbtilesProvider.cpp)
    target_include_directories(uaop-gcs-mbtiles-test PRIVATE src)
    target_link_libraries(uaop-gcs-mbtiles-test PRIVATE Qt6::Gui Qt6::Qml Qt6::Quick Qt6::Sql)
    target_compile_definitions(uaop-gcs-mbtiles-test PRIVATE
        UAOP_MBTILES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../../tests/data/map/world.mbtiles")
    target_compile_features(uaop-gcs-mbtiles-test PRIVATE cxx_std_20)
    add_test(NAME gcs.mbtiles_provider COMMAND uaop-gcs-mbtiles-test)
    set_tests_properties(gcs.mbtiles_provider PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    add_executable(uaop-gcs-flight-report-json-test tests/flight_report_json_test.cpp
                                                     src/models/FlightReportJson.cpp)
    target_include_directories(uaop-gcs-flight-report-json-test PRIVATE src)
    target_link_libraries(uaop-gcs-flight-report-json-test PRIVATE Qt6::Core)
    target_compile_features(uaop-gcs-flight-report-json-test PRIVATE cxx_std_20)
    add_test(NAME gcs.flight_report_json COMMAND uaop-gcs-flight-report-json-test)
endif()
