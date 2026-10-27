import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UAOPGCS

// Post-mission review (ADR-0022): flights analysed by flight-log-engine's FlightMD
// sidecar; for the selected one, score, findings and the parameter sheet. All data
// and wording come from FlightReportsModel (src/models/), this file only lays it out.
Item {
    id: root

    readonly property var reports: flightReports
    readonly property var report: reports.report
    readonly property bool hasReport: report.summary !== undefined
    property string selectedId: ""

    Theme { id: theme }

    Component.onCompleted: reports.refresh()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: theme.spacing2
        spacing: theme.spacing2

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: theme.spacing2

            // ── Flights ──────────────────────────────────────────────────────
            Rectangle {
                Layout.preferredWidth: 260
                Layout.fillHeight: true
                color: theme.bg2
                border.color: theme.stroke
                radius: theme.radius

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: theme.spacing2
                    spacing: theme.spacing1

                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: qsTr("ANALYSED FLIGHTS"); color: theme.fg1; font: theme.panelTitleFont }
                        Item { Layout.fillWidth: true }
                        Button {
                            text: qsTr("REFRESH")
                            font: theme.panelTitleFont
                            enabled: root.reports.state !== "loading"
                            onClicked: root.reports.refresh()
                        }
                    }

                    // Absence and failure are stated, never a blank list (UX_GUIDELINES §3)
                    StatusTag {
                        visible: root.reports.state === "offline" || root.reports.state === "error"
                        tone: root.reports.state === "offline" ? "caution" : "warning"
                        word: root.reports.state === "offline" ? qsTr("OFFLINE") : qsTr("ERROR")
                    }
                    Text {
                        Layout.fillWidth: true
                        visible: text.length > 0
                        text: root.reports.state === "loading" ? qsTr("Loading…")
                              : root.reports.state === "ready" && root.reports.flights.length === 0 ? qsTr("No analysed flights yet.")
                              : root.reports.message
                        color: theme.fg1
                        font: theme.bodyFont2
                        wrapMode: Text.WordWrap
                    }

                    ListView {
                        id: flightList
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        spacing: theme.spacing1
                        model: root.reports.flights
                        keyNavigationEnabled: true

                        delegate: ItemDelegate {
                            required property var modelData
                            width: ListView.view.width
                            highlighted: modelData.reportId === root.selectedId
                            onClicked: {
                                root.selectedId = modelData.reportId
                                root.reports.open(modelData.reportId)
                            }
                            background: Rectangle {
                                color: parent.highlighted ? theme.bg1 : "transparent"
                                border.color: parent.highlighted || parent.visualFocus ? theme.accentColor : theme.stroke
                                radius: theme.radius
                            }
                            contentItem: ColumnLayout {
                                spacing: 2
                                RowLayout {
                                    Text {
                                        Layout.fillWidth: true
                                        text: modelData.fileName
                                        color: theme.fg0
                                        font: theme.bodyFont2
                                        elide: Text.ElideMiddle
                                    }
                                    Text {
                                        text: modelData.scoreText + (modelData.letterGrade && modelData.letterGrade !== "-" ? " " + modelData.letterGrade : "")
                                        color: theme.fg0
                                        font.family: theme.monoFontFamily
                                        font.pixelSize: theme.bodyFont2.pixelSize
                                    }
                                }
                                StatusTag { tone: modelData.tone; word: modelData.statusWord }
                                Text {
                                    text: modelData.analysedAt + (modelData.airframeLabel ? " · " + modelData.airframeLabel : "")
                                    color: theme.fg1
                                    font: theme.labelFont
                                }
                            }
                        }
                    }
                }
            }

            // ── Selected flight ─────────────────────────────────────────────
            ScrollView {
                id: detail
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                contentWidth: availableWidth

                ColumnLayout {
                    width: detail.availableWidth
                    spacing: theme.spacing3

                    Text {
                        visible: !root.hasReport
                        text: qsTr("Select a flight to see its score, findings and parameter sheet.")
                        color: theme.fg1
                        font: theme.bodyFont2
                    }

                    RowLayout {
                        visible: root.hasReport
                        spacing: theme.spacing3
                        Text {
                            text: root.hasReport ? root.report.summary.scoreText : ""
                            color: theme.fg0
                            font: theme.hudValueFont
                        }
                        ColumnLayout {
                            spacing: 2
                            Text {
                                text: root.hasReport ? root.report.summary.scoreLabel + "  " + root.report.summary.letterGrade : ""
                                color: theme.fg0
                                font: theme.bodyFont2
                            }
                            StatusTag {
                                tone: root.hasReport ? root.report.summary.tone : "advisory"
                                word: root.hasReport ? root.report.summary.statusWord : ""
                            }
                        }
                        Item { Layout.fillWidth: true }
                        Text {
                            text: root.hasReport ? qsTr("FlightMD schema %1").arg(root.report.summary.schemaVersion) : ""
                            color: theme.fg1
                            font: theme.labelFont
                        }
                    }

                    Text {
                        Layout.fillWidth: true
                        visible: root.hasReport
                        text: root.hasReport ? root.report.executiveSummary : ""
                        color: theme.fg0
                        font: theme.bodyFont2
                        wrapMode: Text.WordWrap
                    }

                    // Things the parser had to skip or guess: shown, so a clean score isn't over-trusted
                    Repeater {
                        model: root.hasReport ? root.report.parseWarnings : []
                        RowLayout {
                            required property string modelData
                            Layout.fillWidth: true
                            StatusTag { tone: "caution"; word: qsTr("PARSE") }
                            Text { Layout.fillWidth: true; text: modelData; color: theme.fg0; font: theme.bodyFont2; wrapMode: Text.WordWrap }
                        }
                    }

                    Text {
                        visible: root.hasReport
                        text: qsTr("FINDINGS (%1)").arg(root.hasReport ? root.report.findings.length : 0)
                        color: theme.fg1
                        font: theme.panelTitleFont
                    }
                    Repeater {
                        model: root.hasReport ? root.report.findings : []
                        Rectangle {
                            required property var modelData
                            Layout.fillWidth: true
                            implicitHeight: findingBody.implicitHeight + 2 * theme.spacing2
                            color: theme.bg2
                            border.color: theme.stroke
                            radius: theme.radius

                            ColumnLayout {
                                id: findingBody
                                anchors.fill: parent
                                anchors.margins: theme.spacing2
                                spacing: theme.spacing1
                                RowLayout {
                                    StatusTag { tone: modelData.tone; word: modelData.severityWord }
                                    Text { Layout.fillWidth: true; text: modelData.title; color: theme.fg0; font: theme.bodyFont2; elide: Text.ElideRight }
                                    Text {
                                        text: modelData.timeText
                                        color: theme.fg1
                                        font.family: theme.monoFontFamily
                                        font.pixelSize: theme.labelFont.pixelSize
                                    }
                                }
                                Text { Layout.fillWidth: true; text: modelData.plainEnglish; color: theme.fg0; font: theme.bodyFont2; wrapMode: Text.WordWrap; visible: text.length > 0 }
                                Text { Layout.fillWidth: true; text: modelData.recommendation; color: theme.fg1; font: theme.bodyItalicFont; wrapMode: Text.WordWrap; visible: text.length > 0 }
                                Text {
                                    visible: modelData.paramsText.length > 0
                                    text: modelData.paramsText
                                    color: theme.accentText
                                    font.family: theme.monoFontFamily
                                    font.pixelSize: theme.labelFont.pixelSize
                                }
                            }
                        }
                    }

                    Text {
                        visible: root.hasReport
                        text: qsTr("PARAMETER SHEET (%1)").arg(root.hasReport ? root.report.params.length : 0)
                        color: theme.fg1
                        font: theme.panelTitleFont
                    }
                    Text {
                        visible: root.hasReport && root.report.params.length > 0
                        text: qsTr("Recommendations only. Apply them from the Parameters panel, where every write is checked and audited.")
                        color: theme.fg1
                        font: theme.labelFont
                    }
                    Repeater {
                        model: root.hasReport ? root.report.params : []
                        RowLayout {
                            required property var modelData
                            Layout.fillWidth: true
                            spacing: theme.spacing3
                            Text {
                                Layout.preferredWidth: 170
                                text: modelData.name
                                color: theme.fg0
                                font.family: theme.monoFontFamily
                                font.pixelSize: theme.bodyFont2.pixelSize
                            }
                            Text {
                                Layout.preferredWidth: 150
                                text: modelData.current + " → " + modelData.suggested + (modelData.unit ? " " + modelData.unit : "")
                                color: theme.fg0
                                font.family: theme.monoFontFamily
                                font.pixelSize: theme.bodyFont2.pixelSize
                            }
                            Text { Layout.fillWidth: true; text: modelData.reason; color: theme.fg1; font: theme.bodyFont2; wrapMode: Text.WordWrap }
                        }
                    }
                }
            }
        }

        Text {
            Layout.fillWidth: true
            text: qsTr("Analysis by FlightMD · part of the %1: FlightMD · AeroAtlas · UAOP").arg(suiteName)
            color: theme.fg1
            font: theme.labelFont
            horizontalAlignment: Text.AlignRight
        }
    }
}
