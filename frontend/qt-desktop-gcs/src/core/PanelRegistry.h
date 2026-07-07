// Panel registry — the panel-manifest loader (UI_GUIDELINES.md §6): every dockable
// panel is registered here with id, title, QML source, and sizing hints. Plugin
// panels (Phase 4) enter through this same registry, capability-scoped.

#ifndef UAOP_GCS_CORE_PANELREGISTRY_H
#define UAOP_GCS_CORE_PANELREGISTRY_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QVector>

namespace uaop::gcs {

struct PanelDefinition {
    QString id;
    QString title;
    QString source; // relative to ui/ root, e.g. "views/telemetry/TelemetryView.qml"
    int minWidth = 320;
    int minHeight = 240;
};

class PanelRegistry : public QObject {
    Q_OBJECT
    Q_PROPERTY(QStringList panelIds READ panelIds CONSTANT)

public:
    explicit PanelRegistry(QObject* parent = nullptr);

    void registerPanel(const PanelDefinition& def);

    [[nodiscard]] QStringList panelIds() const;

    // QVariantMap {id, title, source, minWidth, minHeight}; empty map for unknown id —
    // the QML side renders an explicit "unknown panel" placeholder, never a blank.
    Q_INVOKABLE [[nodiscard]] QVariantMap panel(const QString& id) const;

private:
    QVector<PanelDefinition> panels_;
};

} // namespace uaop::gcs

#endif // UAOP_GCS_CORE_PANELREGISTRY_H
