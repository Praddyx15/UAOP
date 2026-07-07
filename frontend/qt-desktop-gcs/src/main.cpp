// Wiring only: registry -> workspaces -> engine (PROJECT_STRUCTURE.md §5).
// --selfcheck loads the shell offscreen and exits 0/1 — the ctest smoke gate.

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTimer>
#include <cstring>

#include "TelemetryController.h"
#include "core/PanelRegistry.h"
#include "core/WorkspaceManager.h"
#include "map/MbtilesProvider.h"

#ifndef UAOP_MBTILES_PATH
#define UAOP_MBTILES_PATH ""
#endif

namespace {

bool hasFlag(int argc, char* argv[], const char* flag) {
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], flag) == 0) {
            return true;
        }
    }
    return false;
}

void registerPanels(uaop::gcs::PanelRegistry& registry) {
    registry.registerPanel({"flight", "FLIGHT TELEMETRY", "views/telemetry/TelemetryView.qml", 480, 320});
    registry.registerPanel({"mission", "MISSION", "views/mission/MissionView.qml", 400, 320});
    registry.registerPanel({"compliance", "PRE-FLIGHT COMPLIANCE", "views/compliance/ComplianceView.qml", 360, 320});
    registry.registerPanel({"map", "MAP", "components/MapView.qml", 400, 320});
    // DefenceView.qml stays on disk but unregistered: defense features are
    // Phase 4+ scope (Review R2/F13) — a parked view, not a shipped panel.
}

void registerWorkspaces(uaop::gcs::WorkspaceManager& manager) {
    manager.addWorkspace({"FLIGHT OPS",
                          {{{"flight"}, 3}, {{"map"}, 2}, {{"mission", "compliance"}, 2}}});
    manager.addWorkspace({"REVIEW", {{{"compliance"}, 1}, {{"flight"}, 2}}});
}

} // namespace

int main(int argc, char* argv[]) {
    const bool selfcheck = hasFlag(argc, argv, "--selfcheck");
    if (selfcheck && !qEnvironmentVariableIsSet("QT_QPA_PLATFORM")) {
        qputenv("QT_QPA_PLATFORM", "offscreen");
    }

    QGuiApplication app(argc, argv);
    app.setApplicationName("uaop-gcs");
    app.setOrganizationName("Sixty Motion Aerospace");

    TelemetryController telemetryController;
    uaop::gcs::PanelRegistry panelRegistry;
    uaop::gcs::WorkspaceManager workspaceManager;
    registerPanels(panelRegistry);
    registerWorkspaces(workspaceManager);

    qmlRegisterSingletonInstance("UAOPGCS", 1, 0, "PanelRegistry", &panelRegistry);
    qmlRegisterSingletonInstance("UAOPGCS", 1, 0, "WorkspaceManager", &workspaceManager);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("telemetryController", &telemetryController);
    // Interim MBTiles renderer (ADR-0020); engine takes ownership of the provider.
    engine.addImageProvider("mbtiles", new uaop::gcs::map::MbtilesProvider(
                                           QStringLiteral(UAOP_MBTILES_PATH)));

    const QUrl url(QStringLiteral("qrc:/UAOPGCS/ui/Shell.qml"));
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        []() { QCoreApplication::exit(1); }, Qt::QueuedConnection);
    engine.load(url);

    if (selfcheck) {
        QTimer::singleShot(1200, &app, [&]() {
            QCoreApplication::exit(engine.rootObjects().isEmpty() ? 1 : 0);
        });
    }

    return app.exec();
}
