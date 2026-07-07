#include "PanelRegistry.h"

namespace uaop::gcs {

PanelRegistry::PanelRegistry(QObject* parent) : QObject(parent) {}

void PanelRegistry::registerPanel(const PanelDefinition& def) {
    panels_.append(def);
}

QStringList PanelRegistry::panelIds() const {
    QStringList ids;
    ids.reserve(panels_.size());
    for (const auto& p : panels_) {
        ids.append(p.id);
    }
    return ids;
}

QVariantMap PanelRegistry::panel(const QString& id) const {
    for (const auto& p : panels_) {
        if (p.id == id) {
            return QVariantMap{{"id", p.id},
                               {"title", p.title},
                               {"source", p.source},
                               {"minWidth", p.minWidth},
                               {"minHeight", p.minHeight}};
        }
    }
    return {};
}

} // namespace uaop::gcs
