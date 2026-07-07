#include "WorkspaceManager.h"

namespace uaop::gcs {

WorkspaceManager::WorkspaceManager(QObject* parent) : QObject(parent) {}

void WorkspaceManager::addWorkspace(const Workspace& ws) {
    workspaces_.append(ws);
}

QStringList WorkspaceManager::workspaceNames() const {
    QStringList names;
    names.reserve(workspaces_.size());
    for (const auto& ws : workspaces_) {
        names.append(ws.name);
    }
    return names;
}

void WorkspaceManager::setCurrentIndex(int index) {
    if (index < 0 || index >= workspaces_.size() || index == currentIndex_) {
        return;
    }
    currentIndex_ = index;
    emit currentIndexChanged();
}

QVariantList WorkspaceManager::currentLayout() const {
    QVariantList columns;
    if (currentIndex_ < 0 || currentIndex_ >= workspaces_.size()) {
        return columns;
    }
    for (const auto& col : workspaces_[currentIndex_].columns) {
        columns.append(QVariantMap{{"panels", col.panelIds}, {"stretch", col.stretch}});
    }
    return columns;
}

} // namespace uaop::gcs
