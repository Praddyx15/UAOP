#include "FlightReportsModel.h"

#include <QNetworkReply>
#include <QNetworkRequest>
#include <utility>

#include "FlightReportJson.h"

namespace uaop::gcs::models {

FlightReportsModel::FlightReportsModel(QUrl baseUrl, QObject* parent)
    : QObject(parent), base_(std::move(baseUrl)) {
    // loopback service; a dead sidecar should show "offline" quickly, not after 30 s
    net_.setTransferTimeout(5000);
}

void FlightReportsModel::setState(const QString& state, const QString& message) {
    state_ = state;
    message_ = message;
    emit stateChanged();
}

bool FlightReportsModel::replyOk(QNetworkReply* reply) {
    switch (reply->error()) {
    case QNetworkReply::NoError:
        return true;
    case QNetworkReply::ConnectionRefusedError:
    case QNetworkReply::HostNotFoundError:
    case QNetworkReply::TimeoutError:
    case QNetworkReply::OperationCanceledError:
        setState(QStringLiteral("offline"), tr("Flight log engine not reachable at %1").arg(base_.toString()));
        return false;
    default:
        setState(QStringLiteral("error"), reply->errorString());
        return false;
    }
}

void FlightReportsModel::refresh() {
    setState(QStringLiteral("loading"));
    QUrl url = base_;
    url.setPath(base_.path() + QStringLiteral("/v1/flight-reports"));
    QNetworkReply* reply = net_.get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (!replyOk(reply)) {
            return;
        }
        QString err;
        const QVariantList list = parseFlightList(reply->readAll(), &err);
        if (!err.isEmpty()) {
            setState(QStringLiteral("error"), tr("Unreadable flight list: %1").arg(err));
            return;
        }
        flights_ = list;
        emit flightsChanged();
        setState(QStringLiteral("ready"));
    });
}

void FlightReportsModel::open(const QString& reportId) {
    if (pendingReport_) {
        // the operator clicked another flight; clear first, abort() emits finished right away
        QNetworkReply* old = pendingReport_;
        pendingReport_ = nullptr;
        old->abort();
    }
    setState(QStringLiteral("loading"));
    QUrl url = base_;
    // report ids are uuids; percent-encode anyway rather than trust the list
    url.setPath(base_.path(QUrl::FullyEncoded) + QStringLiteral("/v1/flight-reports/") +
                    QString::fromLatin1(QUrl::toPercentEncoding(reportId)),
                QUrl::TolerantMode);
    QNetworkReply* reply = net_.get(QNetworkRequest(url));
    pendingReport_ = reply;
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() == QNetworkReply::OperationCanceledError && pendingReport_ != reply) {
            return; // superseded by a newer open()
        }
        if (!replyOk(reply)) {
            return;
        }
        QString err;
        const QVariantMap parsed = parseFlightReport(reply->readAll(), &err);
        if (!err.isEmpty()) {
            setState(QStringLiteral("error"), tr("Unreadable report: %1").arg(err));
            return;
        }
        report_ = parsed;
        emit reportChanged();
        setState(QStringLiteral("ready"));
    });
}

} // namespace uaop::gcs::models
