// Parsing of the analysis sidecar's JSON for the post-mission panel (FlightReportJson.h).
// GCS is DAL D-equivalent (COMPLIANCE.md §A.2); no RTM/@req trace obligation applies here.

#include <cassert>
#include <cstdlib>
#include <iostream>

#include <QStringList>
#include <QVariant>

#include "models/FlightReportJson.h"

namespace {

using namespace uaop::gcs::models;

const QByteArray kList = R"({"reports": [
  {"report_id": "r1", "file_name": "a.ulg", "overall_score": 71.5, "letter_grade": "C",
   "critical_count": 1, "warning_count": 2},
  {"report_id": "r2", "file_name": "b.ulg", "overall_score": 93, "letter_grade": "A",
   "critical_count": 0, "warning_count": 0},
  {"report_id": "r3", "file_name": "c.bin", "overall_score": null, "letter_grade": "-",
   "critical_count": 0, "warning_count": 0}
], "next_page_token": ""})";

const QByteArray kReport = R"({"report": {
  "summary": {"report_id": "r1", "overall_score": 71.5, "critical_count": 1, "warning_count": 0},
  "executive_summary": "Roll oscillation.",
  "findings": [
    {"id": "a1", "severity": "critical", "title": "Roll oscillation", "confidence": 0.9,
     "t_start_s": 40.0, "t_end_s": 52.0, "params": ["MC_ROLLRATE_P", "MC_ROLLRATE_D"]},
    {"id": "b2", "severity": "warning", "title": "Voltage sag", "confidence": 0.6,
     "t_start_s": 200.5, "t_end_s": null, "params": []},
    {"id": "c3", "severity": "info", "title": "No time", "confidence": 0.3,
     "t_start_s": null, "t_end_s": null, "params": []}
  ],
  "param_change_sheet": [
    {"param_name": "MC_ROLLRATE_P", "current_value": 0.18, "suggested_value": 0.15,
     "unit": "", "change_direction": "decrease", "reason": "oscillation"}
  ],
  "parse_warnings": ["2 GPS samples skipped"]
}})";

void test_tones() {
    assert(toneForSeverity("critical") == "warning");
    assert(toneForSeverity("warning") == "caution");
    assert(toneForSeverity("info") == "advisory");
    assert(toneForSeverity("good") == "ok");
    assert(toneForSeverity("nonsense") == "advisory");
}

void test_list() {
    QString err;
    const QVariantList list = parseFlightList(kList, &err);
    assert(err.isEmpty());
    assert(list.size() == 3);

    const QVariantMap first = list[0].toMap();
    assert(first["reportId"].toString() == "r1");
    assert(first["scoreText"].toString() == "72");
    assert(first["statusWord"].toString() == "CRITICAL");
    assert(first["tone"].toString() == "warning");

    assert(list[1].toMap()["statusWord"].toString() == "GOOD");
    assert(list[1].toMap()["tone"].toString() == "ok");

    // not assessed must not read as a clean flight
    const QVariantMap third = list[2].toMap();
    assert(third["scoreText"].toString() == QString::fromUtf8("—"));
    assert(third["statusWord"].toString() == "NOT ASSESSED");
}

void test_report() {
    QString err;
    const QVariantMap r = parseFlightReport(kReport, &err);
    assert(err.isEmpty());
    assert(r["executiveSummary"].toString() == "Roll oscillation.");

    const QVariantList findings = r["findings"].toList();
    assert(findings.size() == 3);
    const QVariantMap f0 = findings[0].toMap();
    assert(f0["severityWord"].toString() == "CRITICAL");
    assert(f0["tone"].toString() == "warning");
    assert(f0["confidencePct"].toInt() == 90);
    assert(f0["timeText"].toString() == QString::fromUtf8("40.0–52.0 s"));
    assert(f0["paramsText"].toString() == "MC_ROLLRATE_P, MC_ROLLRATE_D");
    assert(findings[1].toMap()["timeText"].toString() == "200.5 s");
    assert(findings[2].toMap()["timeText"].toString().isEmpty());

    const QVariantMap p0 = r["params"].toList()[0].toMap();
    assert(p0["name"].toString() == "MC_ROLLRATE_P");
    assert(p0["current"].toString() == "0.18");
    assert(p0["suggested"].toString() == "0.15");

    assert(r["parseWarnings"].toList().size() == 1);
}

void test_malformed_input() {
    QString err;
    assert(parseFlightList("not json", &err).isEmpty());
    assert(!err.isEmpty());

    err.clear();
    assert(parseFlightList(R"({"reports": 3})", &err).isEmpty());
    assert(!err.isEmpty());

    err.clear();
    assert(parseFlightReport(R"({"other": {}})", &err).isEmpty());
    assert(!err.isEmpty());
}

} // namespace

int main() {
    test_tones();
    test_list();
    test_report();
    test_malformed_input();
    std::cout << "flight_report_json_test: all assertions passed\n";
    return EXIT_SUCCESS;
}
