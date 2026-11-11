// Post-mission panel model: analysed flights and the selected report, fetched from
// flight-log-engine's analysis sidecar (GET /v1/flight-reports[/{id}], ADR-0022).
// Read-only: applying a recommended parameter is a command-class write and belongs
// to the Parameters/Tuning panels (UI_GUIDELINES §7, M4.6), not here.

#ifndef UAOP_GCS_MODELS_FLIGHTREPORTSMODEL_H
#define UAOP_GCS_MODELS_FLIGHTREPORTSMODEL_H

#include <QNetworkAccessManager>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QUrl>
#include <QVariantList>
#include <QVariantMap>

class QNetworkReply;

namespace uaop::gcs::models {

class FlightReportsModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList flights READ flights NOTIFY flightsChanged)
    Q_PROPERTY(QVariantMap report READ report NOTIFY reportChanged)
    // idle | loading | ready | offline | error; the panel shows each one explicitly
    Q_PROPERTY(QString state READ state NOTIFY stateChanged)
    Q_PROPERTY(QString message READ message NOTIFY stateChanged)

public:
    explicit FlightReportsModel(QUrl baseUrl, QObject* parent = nullptr);

    [[nodiscard]] QVariantList flights() const { return flights_; }
    [[nodiscard]] QVariantMap report() const { return report_; }
    [[nodiscard]] QString state() const { return state_; }
    [[nodiscard]] QString message() const { return message_; }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void open(const QString& reportId);

signals:
    void flightsChanged();
    void reportChanged();
    void stateChanged();

private:
    void setState(const QString& state, const QString& message = {});
    // true when the reply carried a body to parse; otherwise state is offline/error
    bool replyOk(QNetworkReply* reply);

    QNetworkAccessManager net_;
    QUrl base_;
    QVariantList flights_;
    QVariantMap report_;
    QString state_ = QStringLiteral("idle");
    QString message_;
    QPointer<QNetworkReply> pendingReport_;
};

} // namespace uaop::gcs::models

#endif // UAOP_GCS_MODELS_FLIGHTREPORTSMODEL_H
