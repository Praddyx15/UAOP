import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UAOPGCS
import "styles"
import "views/telemetry"
import "views/compliance"
import "views/defence"
import "views/mission"

ApplicationWindow {
    id: window
    visible: true
    width: 1024
    height: 768
    title: "UAOP - Tactical Ground Control Station"
    color: theme.background

    Theme { id: theme }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // 1. Sidebar Navigation Menu
        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: 240
            color: theme.panelBg
            border.color: theme.borderPrimary
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 15
                spacing: 10

                // Header / Branding Logo
                Row {
                    Layout.fillWidth: true
                    spacing: 12
                    
                    Rectangle {
                        width: 24
                        height: 24
                        color: theme.accent
                        radius: 4
                        Text {
                            text: "U"
                            color: theme.background
                            font.bold: true
                            font.pixelSize: 14
                            anchors.centerIn: parent
                        }
                    }

                    Text {
                        text: "UAOP GCS"
                        color: theme.textPrimary
                        font: theme.headerFont
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: theme.borderPrimary
                }

                // Nav Buttons
                Column {
                    Layout.fillWidth: true
                    spacing: 8

                    NavigationButton {
                        text: "Flight Telemetry"
                        active: viewStack.currentIndex === 0
                        onClicked: viewStack.currentIndex = 0
                    }

                    NavigationButton {
                        text: "Pre-Flight Compliance"
                        active: viewStack.currentIndex === 1
                        onClicked: viewStack.currentIndex = 1
                    }

                    NavigationButton {
                        text: "Defence & Jamming"
                        active: viewStack.currentIndex === 2
                        onClicked: viewStack.currentIndex = 2
                    }

                    NavigationButton {
                        text: "Tactical Mission Planner"
                        active: viewStack.currentIndex === 3
                        onClicked: viewStack.currentIndex = 3
                    }
                }

                Item { Layout.fillHeight: true } // Spacer

                // Tactical Jammer Toggle Button (for LinkedIn demo verification)
                Button {
                    Layout.fillWidth: true
                    text: telemetryController.signalJamming > 50 ? "DEACTIVATE JAMMING" : "SIMULATE RF JAMMING"
                    font: theme.titleFont
                    contentItem: Text {
                        text: parent.text
                        font: parent.font
                        color: "#FFFFFF"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        implicitHeight: 45
                        color: telemetryController.signalJamming > 50 ? theme.greenAccent : theme.redAccent
                        radius: 6
                    }
                    onClicked: {
                        telemetryController.toggleJammingSimulation();
                    }
                }
            }
        }

        // 2. Main Content Area (Top bar + Active View Layout)
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            // Top Status Panel (Military Warning Bar)
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 70
                color: telemetryController.readyToFly ? "#112C24" : "#2E1920"
                border.color: telemetryController.readyToFly ? theme.greenAccent : theme.redAccent
                border.width: 1

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 20
                    anchors.rightMargin: 20

                    // Binary health lamp
                    Rectangle {
                        width: 16
                        height: 16
                        radius: 8
                        color: telemetryController.readyToFly ? theme.greenAccent : theme.redAccent
                    }

                    Text {
                        text: telemetryController.readyToFly ? "SYSTEM STATUS: READY TO FLY" : "SYSTEM STATUS: " + telemetryController.statusMessage.toUpperCase()
                        color: telemetryController.readyToFly ? theme.greenAccent : theme.redAccent
                        font: theme.headerFont
                        Layout.fillWidth: true
                    }

                    // Tactical Link status flags
                    Row {
                        spacing: 20
                        
                        // E2EE indicator
                        Rectangle {
                            width: 100
                            height: 26
                            radius: 4
                            color: telemetryController.e2eeEnabled ? "#112C24" : "#2E1920"
                            border.color: telemetryController.e2eeEnabled ? theme.greenAccent : theme.redAccent
                            border.width: 1

                            Text {
                                text: "E2EE ACTIVE"
                                color: telemetryController.e2eeEnabled ? theme.greenAccent : theme.redAccent
                                font: theme.telemetryFont
                                font.pixelSize: 10
                                anchors.centerIn: parent
                            }
                        }

                        // FHSS indicator
                        Rectangle {
                            width: 100
                            height: 26
                            radius: 4
                            color: telemetryController.fhssSyncActive ? "#112C24" : "#2E1920"
                            border.color: telemetryController.fhssSyncActive ? theme.greenAccent : theme.redAccent
                            border.width: 1

                            Text {
                                text: "FHSS SYNC"
                                color: telemetryController.fhssSyncActive ? theme.greenAccent : theme.redAccent
                                font: theme.telemetryFont
                                font.pixelSize: 10
                                anchors.centerIn: parent
                            }
                        }
                    }
                }
            }

            // StackLayout for loading active subviews
            StackLayout {
                id: viewStack
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentIndex: 0

                TelemetryView {}
                ComplianceView {}
                DefenceView {}
                MissionView {}
            }
        }
    }
}
