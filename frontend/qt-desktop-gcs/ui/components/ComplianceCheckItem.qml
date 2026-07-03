import QtQuick
import UAOPGCS

Item {
    id: root
    width: parent.width
    height: 60

    property string label: ""
    property string status: ""
    property string details: ""
    property bool passed: false

    Theme { id: theme }

    Row {
        anchors.fill: parent
        spacing: 15

        // Left Status Icon Indicator
        Rectangle {
            width: 28
            height: 28
            radius: 14
            color: root.passed ? "#112C24" : "#2E1920"
            border.color: root.passed ? theme.greenAccent : theme.redAccent
            border.width: 1
            anchors.verticalCenter: parent.verticalCenter

            Text {
                text: root.passed ? "✓" : "✗"
                color: root.passed ? theme.greenAccent : theme.redAccent
                font.bold: true
                font.pixelSize: 14
                anchors.centerIn: parent
            }
        }

        // Checklist Text Details
        Column {
            width: parent.width - 150
            anchors.verticalCenter: parent.verticalCenter
            spacing: 3

            Text {
                text: root.label
                color: theme.textPrimary
                font: theme.titleFont
            }

            Text {
                text: root.details
                color: theme.textSecondary
                font: theme.bodyFont
                elide: Text.ElideRight
            }
        }

        // Status Label Box
        Rectangle {
            width: 90
            height: 26
            radius: 4
            color: root.passed ? "#112C24" : "#2E1920"
            border.color: root.passed ? theme.greenAccent : theme.redAccent
            border.width: 1
            anchors.verticalCenter: parent.verticalCenter

            Text {
                text: root.status
                color: root.passed ? theme.greenAccent : theme.redAccent
                font: theme.telemetryFont
                anchors.centerIn: parent
            }
        }
    }
}
