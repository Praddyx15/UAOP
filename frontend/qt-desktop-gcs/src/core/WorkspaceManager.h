// Workspace manager — named panel layouts switched atomically (UI_GUIDELINES.md §2).
// A layout is a list of columns; each column is a stack of panel ids (tabbed when >1).
// Layout persistence and drag-rearrange land with M6; the model here is already the
// shape those features mutate.

#ifndef UAOP_GCS_CORE_WORKSPACEMANAGER_H
#define UAOP_GCS_CORE_WORKSPACEMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QVector>

namespace uaop::gcs {

struct WorkspaceColumn {
    QStringList panelIds;
    int stretch = 1;
};

struct Workspace {
    QString name;
    QVector<WorkspaceColumn> columns;
};

class WorkspaceManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QStringList workspaceNames READ workspaceNames CONSTANT)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(QVariantList currentLayout READ currentLayout NOTIFY currentIndexChanged)

public:
    explicit WorkspaceManager(QObject* parent = nullptr);

    void addWorkspace(const Workspace& ws);

    [[nodiscard]] QStringList workspaceNames() const;

    [[nodiscard]] int currentIndex() const { return currentIndex_; }

    void setCurrentIndex(int index);

    // List of QVariantMap {panels: QStringList, stretch: int} — one entry per column.
    [[nodiscard]] QVariantList currentLayout() const;

signals:
    void currentIndexChanged();

private:
    QVector<Workspace> workspaces_;
    int currentIndex_ = 0;
};

} // namespace uaop::gcs

#endif // UAOP_GCS_CORE_WORKSPACEMANAGER_H
