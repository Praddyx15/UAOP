// JSON from flight-log-engine's analysis sidecar (log.proto ListFlightReports /
// GetFlightReport field names) -> the QVariant shapes the post-mission panel binds to.
// Pure QtCore so it is unit-testable without a network or a QML engine.

#ifndef UAOP_GCS_MODELS_FLIGHTREPORTJSON_H
#define UAOP_GCS_MODELS_FLIGHTREPORTJSON_H

#include <QByteArray>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

namespace uaop::gcs::models {

// FlightMD severity -> the GCS semantic tone (STYLE_GUIDE §2): critical is WARNING red,
// warning is CAUTION amber, info is ADVISORY cyan, good is nominal green. Unknown -> "advisory".
[[nodiscard]] QString toneForSeverity(const QString& severity);

// "40.0–52.0 s", "200.5 s", or "" when the finding has no time.
[[nodiscard]] QString timeWindowText(const QVariant& start, const QVariant& end);

// {"reports": [...]} -> list of summary maps, each with scoreText and tone added.
// Returns an empty list and sets *error on malformed input.
[[nodiscard]] QVariantList parseFlightList(const QByteArray& json, QString* error);

// {"report": {...}} -> {summary, executiveSummary, findings[], params[], parseWarnings[]}.
[[nodiscard]] QVariantMap parseFlightReport(const QByteArray& json, QString* error);

} // namespace uaop::gcs::models

#endif // UAOP_GCS_MODELS_FLIGHTREPORTJSON_H
