// Wiring only: registry -> workspaces -> engine (PROJECT_STRUCTURE.md §5).
// --selfcheck loads the shell offscreen and exits 0/1 — the ctest smoke gate.

#include <QDebug>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTimer>
#include <array>
#include <cstring>

#include "TelemetryController.h"
#include "core/PanelRegistry.h"
#include "core/Suite.h"
#include "core/WorkspaceManager.h"
#include "map/MbtilesProvider.h"
#include "models/FlightReportsModel.h"

#ifndef UAOP_MBTILES_PATH
#define UAOP_MBTILES_PATH ""
#endif

#ifndef UAOP_FLIGHT_LOG_URL_DEFAULT
#define UAOP_FLIGHT_LOG_URL_DEFAULT "http://127.0.0.1:8095"
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
    registry.registerPanel({"post-mission", "POST-MISSION", "views/logs/PostMissionView.qml", 520, 320});
    // DefenceView.qml stays on disk but unregistered: defense features are
    // Phase 4+ scope (Review R2/F13) — a parked view, not a shipped panel.
}

void registerWorkspaces(uaop::gcs::WorkspaceManager& manager) {
    manager.addWorkspace({"FLIGHT OPS",
                          {{{"flight"}, 3}, {{"map"}, 2}, {{"mission", "compliance"}, 2}}});
    manager.addWorkspace({"REVIEW", {{{"compliance"}, 1}, {{"post-mission", "flight"}, 2}}});
}

// Faces registered under their family names ("Geist", "Geist Mono"); Qt picks the
// closest face for a weight/italic request.
void loadBundledFonts() {
    static constexpr std::array<const char*, 11> kFaces = {
        ":/fonts/Geist-Light.ttf",       ":/fonts/Geist-LightItalic.ttf",
        ":/fonts/Geist-Regular.ttf",     ":/fonts/Geist-Italic.ttf",
        ":/fonts/Geist-Medium.ttf",      ":/fonts/Geist-MediumItalic.ttf",
        ":/fonts/Geist-Bold.ttf",        ":/fonts/Geist-BoldItalic.ttf",
        ":/fonts/GeistMono-Regular.ttf", ":/fonts/GeistMono-Medium.ttf",
        ":/fonts/GeistMono-Bold.ttf",
    };
    for (const char* path : kFaces) {
        if (QFontDatabase::addApplicationFont(QString::fromLatin1(path)) == -1) {
            qWarning("failed to load bundled font: %s", path);
        }
    }
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

    loadBundledFonts();

    TelemetryController telemetryController;
    // TODO(M5.5): go through api-gateway once the GCS has a gateway session; until
    // then the sidecar is read directly on loopback
    const QString flightLogUrl = qEnvironmentVariableIsSet("UAOP_FLIGHT_LOG_URL")
                                     ? qEnvironmentVariable("UAOP_FLIGHT_LOG_URL")
                                     : QStringLiteral(UAOP_FLIGHT_LOG_URL_DEFAULT);
    uaop::gcs::models::FlightReportsModel flightReports{QUrl(flightLogUrl)};
    uaop::gcs::PanelRegistry panelRegistry;
    uaop::gcs::WorkspaceManager workspaceManager;
    registerPanels(panelRegistry);
    registerWorkspaces(workspaceManager);

    qmlRegisterSingletonInstance("UAOPGCS", 1, 0, "PanelRegistry", &panelRegistry);
    qmlRegisterSingletonInstance("UAOPGCS", 1, 0, "WorkspaceManager", &workspaceManager);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("telemetryController", &telemetryController);
    engine.rootContext()->setContextProperty("flightReports", &flightReports);
    engine.rootContext()->setContextProperty("suiteName", QString::fromUtf8(uaop::gcs::SUITE_NAME));
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
