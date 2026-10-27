#include "FlightReportJson.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QStringList>

namespace uaop::gcs::models {

namespace {

QJsonObject parseObject(const QByteArray& json, QString* error) {
    QJsonParseError err{};
    const QJsonDocument doc = QJsonDocument::fromJson(json, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        if (error) {
            *error = err.error != QJsonParseError::NoError ? err.errorString() : QStringLiteral("not a JSON object");
        }
        return {};
    }
    return doc.object();
}

QString number(const QJsonValue& v) {
    return v.isDouble() ? QString::number(v.toDouble(), 'g', 6) : QStringLiteral("—");
}

QVariantMap summaryMap(const QJsonObject& s) {
    const int critical = s.value("critical_count").toInt();
    const int warning = s.value("warning_count").toInt();
    const QJsonValue score = s.value("overall_score");
    const bool assessed = score.isDouble();

    QString tone = QStringLiteral("ok");
    QString word = QStringLiteral("GOOD");
    if (critical > 0) {
        tone = toneForSeverity("critical");
        word = QStringLiteral("CRITICAL");
    } else if (warning > 0) {
        tone = toneForSeverity("warning");
        word = QStringLiteral("WARNING");
    } else if (!assessed) {
        // nothing assessed is not the same as a clean flight
        tone = QStringLiteral("advisory");
        word = QStringLiteral("NOT ASSESSED");
    }

    return QVariantMap{
        {"reportId", s.value("report_id").toString()},
        {"fileName", s.value("file_name").toString()},
        {"vehicleId", s.value("vehicle_id").toString()},
        {"airframeLabel", s.value("airframe_label").toString()},
        {"analysedAt", s.value("analysed_at").toString()},
        {"schemaVersion", s.value("schema_version").toString()},
        {"scoreText", assessed ? QString::number(score.toDouble(), 'f', 0) : QStringLiteral("—")},
        {"scoreLabel", s.value("score_label").toString()},
        {"letterGrade", s.value("letter_grade").toString()},
        {"criticalCount", critical},
        {"warningCount", warning},
        {"tone", tone},
        {"statusWord", word},
    };
}

} // namespace

QString toneForSeverity(const QString& severity) {
    if (severity == QLatin1String("critical")) {
        return QStringLiteral("warning");
    }
    if (severity == QLatin1String("warning")) {
        return QStringLiteral("caution");
    }
    if (severity == QLatin1String("good")) {
        return QStringLiteral("ok");
    }
    return QStringLiteral("advisory");
}

QString timeWindowText(const QVariant& start, const QVariant& end) {
    bool okStart = false;
    bool okEnd = false;
    const double t0 = start.toDouble(&okStart);
    const double t1 = end.toDouble(&okEnd);
    if (!okStart || start.isNull()) {
        return {};
    }
    if (!okEnd || end.isNull() || t1 <= t0) {
        return QStringLiteral("%1 s").arg(t0, 0, 'f', 1);
    }
    return QStringLiteral("%1–%2 s").arg(t0, 0, 'f', 1).arg(t1, 0, 'f', 1);
}

QVariantList parseFlightList(const QByteArray& json, QString* error) {
    const QJsonObject root = parseObject(json, error);
    if (!root.value("reports").isArray()) {
        if (error && error->isEmpty()) {
            *error = QStringLiteral("no reports array");
        }
        return {};
    }
    QVariantList out;
    for (const auto& v : root.value("reports").toArray()) {
        if (v.isObject()) {
            out.append(summaryMap(v.toObject()));
        }
    }
    return out;
}

QVariantMap parseFlightReport(const QByteArray& json, QString* error) {
    const QJsonObject r = parseObject(json, error).value("report").toObject();
    if (r.isEmpty()) {
        if (error && error->isEmpty()) {
            *error = QStringLiteral("no report object");
        }
        return {};
    }

    QVariantList findings;
    for (const auto& v : r.value("findings").toArray()) {
        const QJsonObject f = v.toObject();
        const QString severity = f.value("severity").toString();
        QStringList params;
        for (const auto& p : f.value("params").toArray()) {
            params.append(p.toString());
        }
        findings.append(QVariantMap{
            {"id", f.value("id").toString()},
            {"title", f.value("title").toString()},
            {"severityWord", severity.toUpper()},
            {"tone", toneForSeverity(severity)},
            {"plainEnglish", f.value("plain_english").toString()},
            {"recommendation", f.value("recommendation").toString()},
            {"confidencePct", qRound(f.value("confidence").toDouble() * 100.0)},
            {"timeText", timeWindowText(f.value("t_start_s").toVariant(), f.value("t_end_s").toVariant())},
            {"paramsText", params.join(QStringLiteral(", "))},
        });
    }

    QVariantList sheet;
    for (const auto& v : r.value("param_change_sheet").toArray()) {
        const QJsonObject p = v.toObject();
        sheet.append(QVariantMap{
            {"name", p.value("param_name").toString()},
            {"current", number(p.value("current_value"))},
            {"suggested", number(p.value("suggested_value"))},
            {"unit", p.value("unit").toString()},
            {"direction", p.value("change_direction").toString()},
            {"reason", p.value("reason").toString()},
        });
    }

    QVariantList warnings;
    for (const auto& v : r.value("parse_warnings").toArray()) {
        warnings.append(v.toString());
    }

    return QVariantMap{
        {"summary", summaryMap(r.value("summary").toObject())},
        {"executiveSummary", r.value("executive_summary").toString()},
        {"findings", findings},
        {"params", sheet},
        {"parseWarnings", warnings},
    };
}

} // namespace uaop::gcs::models
