#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "TelemetryController.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // Register TelemetryController as a singleton or context property
    TelemetryController telemetryController;

    QQmlApplicationEngine engine;
    
    // Set context properties for QML bindings
    engine.rootContext()->setContextProperty("telemetryController", &telemetryController);

    const QUrl url(QStringLiteral("qrc:/UAOPGCS/ui/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
