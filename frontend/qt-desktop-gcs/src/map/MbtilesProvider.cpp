#include "MbtilesProvider.h"

#include <QFileInfo>
#include <QPainter>
#include <QSqlQuery>
#include <QUuid>

#include "SlippyMath.h"

namespace uaop::gcs::map {

MbtilesProvider::MbtilesProvider(const QString& mbtilesPath)
    : QQuickImageProvider(QQuickImageProvider::Image),
      connectionName_("uaop-mbtiles-" + QUuid::createUuid().toString(QUuid::Id128)) {
    db_ = QSqlDatabase::addDatabase("QSQLITE", connectionName_);
    db_.setDatabaseName(mbtilesPath);

    if (!QFileInfo::exists(mbtilesPath) || !db_.open()) {
        return; // isOpen() reports the failure; requestImage() degrades to placeholders
    }

    QSqlQuery meta(db_);
    meta.prepare("SELECT value FROM metadata WHERE name = ?");
    meta.addBindValue("minzoom");
    if (meta.exec() && meta.next()) {
        minZoom_ = meta.value(0).toInt();
    }
    meta.addBindValue("maxzoom");
    if (meta.exec() && meta.next()) {
        maxZoom_ = meta.value(0).toInt();
    }
}

MbtilesProvider::~MbtilesProvider() {
    db_ = QSqlDatabase(); // release handle before removeDatabase
    QSqlDatabase::removeDatabase(connectionName_);
}

bool MbtilesProvider::isOpen() const {
    return db_.isOpen();
}

QImage MbtilesProvider::placeholderTile(const QString& reason) {
    QImage img(256, 256, QImage::Format_ARGB32);
    img.fill(QColor("#1A2027")); // theme.bg2 — visually distinct from real tiles
    QPainter p(&img);
    p.setPen(QColor("#5C6773"));
    p.drawRect(0, 0, 255, 255);
    p.drawText(img.rect(), Qt::AlignCenter | Qt::TextWordWrap, reason);
    return img;
}

QImage MbtilesProvider::requestImage(const QString& id, QSize* size, const QSize&) {
    const QStringList parts = id.split('/');
    bool okZ = false, okX = false, okY = false;
    const int z = parts.size() > 0 ? parts[0].toInt(&okZ) : 0;
    const qint64 x = parts.size() > 1 ? parts[1].toLongLong(&okX) : 0;
    const qint64 y = parts.size() > 2 ? parts[2].toLongLong(&okY) : 0;

    if (!okZ || !okX || !okY || !db_.isOpen()) {
        const QImage ph = placeholderTile(db_.isOpen() ? "invalid tile id" : "map data unavailable");
        if (size) *size = ph.size();
        return ph;
    }

    const qint64 tmsRow = xyzToTmsRow(y, z);
    QSqlQuery q(db_);
    q.prepare("SELECT tile_data FROM tiles WHERE zoom_level = ? AND tile_column = ? "
              "AND tile_row = ?");
    q.addBindValue(z);
    q.addBindValue(x);
    q.addBindValue(tmsRow);

    QImage img;
    if (q.exec() && q.next()) {
        img.loadFromData(q.value(0).toByteArray());
    }
    if (img.isNull()) {
        img = placeholderTile(QString("no tile\nz%1/%2/%3").arg(z).arg(x).arg(y));
    }
    if (size) *size = img.size();
    return img;
}

} // namespace uaop::gcs::map
