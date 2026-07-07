// MBTiles-backed QQuickImageProvider (interim map renderer — ADR-0020).
// QML requests "image://mbtiles/<z>/<x>/<y>"; this class resolves it against a
// local MBTiles SQLite file via QSqlDatabase(QSQLITE) — no third-party
// dependency, bundled with Qt. Swappable behind MapConfig without touching
// QML call sites (ADR-0008's stated design).

#ifndef UAOP_GCS_MAP_MBTILESPROVIDER_H
#define UAOP_GCS_MAP_MBTILESPROVIDER_H

#include <QQuickImageProvider>
#include <QSqlDatabase>
#include <QString>

namespace uaop::gcs::map {

class MbtilesProvider final : public QQuickImageProvider {
public:
    explicit MbtilesProvider(const QString& mbtilesPath);
    ~MbtilesProvider() override;

    // id is "<z>/<x>/<y>" (XYZ); returns a placeholder (not null) image on any
    // miss so a broken tile never crashes the renderer or shows undefined
    // content (UX_GUIDELINES.md §3 absence discipline, applied to map tiles).
    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override;

    [[nodiscard]] bool isOpen() const;
    [[nodiscard]] int minZoom() const { return minZoom_; }
    [[nodiscard]] int maxZoom() const { return maxZoom_; }

private:
    static QImage placeholderTile(const QString& reason);

    QSqlDatabase db_;
    QString connectionName_;
    int minZoom_ = 0;
    int maxZoom_ = 0;
};

} // namespace uaop::gcs::map

#endif // UAOP_GCS_MAP_MBTILESPROVIDER_H
