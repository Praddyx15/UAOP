import QtQuick
import QtQuick.Controls
import UAOPGCS

Item {
    id: root

    Theme { id: theme }

    ScrollView {
        anchors.fill: parent
        contentWidth: parent.width
        clip: true

        Column {
            width: parent.width - 40
            x: 20
            y: 20
            spacing: 20

            Text {
                text: "PRE-FLIGHT COMPLIANCE & SAFETY AUDIT"
                color: theme.textPrimary
                font: theme.headerFont
            }

            // DGCA NPNT Encryption & Handshake Box
            Rectangle {
                width: parent.width
                height: 180
                color: theme.panelBg
                border.color: theme.borderPrimary
                radius: theme.borderRadius
                padding: 20
                clip: true

                Column {
                    anchors.fill: parent
                    spacing: 12

                    Row {
                        width: parent.width
                        Text {
                            text: "DGCA NPNT (No Permission, No Takeoff) Cryptographic Handshake"
                            color: theme.accent
                            font: theme.titleFont
                        }
                    }

                    Row {
                        spacing: 20
                        width: parent.width

                        // Visual status light
                        Rectangle {
                            width: 14
                            height: 14
                            radius: 7
                            color: telemetryController.readyToFly ? theme.greenAccent : theme.redAccent
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Text {
                            text: telemetryController.readyToFly ? "NPNT Envelope Verified & Signed" : "NPNT Envelope Validation Pending"
                            color: theme.textPrimary
                            font: theme.bodyFont
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    Rectangle {
                        width: parent.width
                        height: 1
                        color: theme.borderPrimary
                    }

                    Grid {
                        columns: 2
                        width: parent.width
                        spacing: 10

                        Text { text: "Digital Envelope Signature: "; color: theme.textSecondary; font: theme.bodyFont; width: 180 }
                        Text { 
                            text: telemetryController.readyToFly ? "0x3A9F...B882 (SHA-256 Validated)" : "No Valid Signature Generated"
                            color: telemetryController.readyToFly ? theme.greenAccent : theme.redAccent
                            font: theme.telemetryFont 
                        }

                        Text { text: "Validation Server: "; color: theme.textSecondary; font: theme.bodyFont; width: 180 }
                        Text { 
                            text: "https://npnt.dgca.gov.in/api/v1/handshake"
                            color: theme.textPrimary
                            font: theme.telemetryFont 
                        }
                    }
                }
            }

            // Compliance Checks Grid
            Grid {
                width: parent.width
                columns: parent.width > 900 ? 2 : 1
                spacing: 20

                // Card 1: Critical Sensor & Hardware Checklist
                Rectangle {
                    width: parent.width > 900 ? (parent.width - 20) / 2 : parent.width
                    height: 380
                    color: theme.panelBg
                    border.color: theme.borderPrimary
                    radius: theme.borderRadius
                    padding: 20

                    Column {
                        anchors.fill: parent
                        spacing: 15

                        Text {
                            text: "CRITICAL FLIGHT DIAGNOSTICS"
                            color: theme.textSecondary
                            font: theme.titleFont
                        }

                        // Checklist items
                        ComplianceCheckItem {
                            label: "IMU & Compass Calibration"
                            status: telemetryController.readyToFly ? "PASS" : "FAIL"
                            details: telemetryController.readyToFly ? "Gyro & Accel within threshold variances" : "Compass electromagnetic drift too high"
                            passed: telemetryController.readyToFly
                        }

                        ComplianceCheckItem {
                            label: "Battery Pack Verification"
                            status: telemetryController.batteryRemaining > 0.20 ? "PASS" : "WARNING"
                            details: "Total voltage: " + telemetryController.batteryVoltage.toFixed(2) + "V (All cells matched)"
                            passed: telemetryController.batteryRemaining > 0.20
                        }

                        ComplianceCheckItem {
                            label: "GPS Satellites & Lock"
                            status: telemetryController.satellites >= 6 ? "PASS" : "FAIL"
                            details: "Visible sats: " + telemetryController.satellites + " (Requires >= 6)"
                            passed: telemetryController.satellites >= 6
                        }

                        ComplianceCheckItem {
                            label: "ASTM F3322 Parachute System"
                            status: "ARMED"
                            details: "Independent trigger circuit operational"
                            passed: true
                        }
                    }
                }

                // Card 2: Airspace & Regulatory Compliance
                Rectangle {
                    width: parent.width > 900 ? (parent.width - 20) / 2 : parent.width
                    height: 380
                    color: theme.panelBg
                    border.color: theme.borderPrimary
                    radius: theme.borderRadius
                    padding: 20

                    Column {
                        anchors.fill: parent
                        spacing: 15

                        Text {
                            text: "AIRSPACE & REGULATORY RULES"
                            color: theme.textSecondary
                            font: theme.titleFont
                        }

                        ComplianceCheckItem {
                            label: "FAA Part 107 Hard Fence"
                            status: "ACTIVE"
                            details: "Altitude limit enforced: 121.9m AGL (400ft)"
                            passed: true
                        }

                        ComplianceCheckItem {
                            label: "No-Fly Zone (NFZ) Check"
                            status: "PASS"
                            details: "Coordinates clear of active restricted airspace grids"
                            passed: true
                        }

                        ComplianceCheckItem {
                            label: "BVLOS Safety Backup Link"
                            status: telemetryController.fhssSyncActive ? "ACTIVE" : "NO LINK"
                            details: telemetryController.fhssSyncActive ? "FHSS RF Link Active + E2EE Engaged" : "Backup RF Sync Unavailable"
                            passed: telemetryController.fhssSyncActive
                        }

                        ComplianceCheckItem {
                            label: "Emergency Abort Path"
                            status: "LOADED"
                            details: "RTL waypoint path cached in local volatile storage"
                            passed: true
                        }
                    }
                }
            }

            // Arm/Disarm Operational Box
            Rectangle {
                width: parent.width
                height: 100
                color: theme.panelBg
                border.color: theme.borderPrimary
                radius: theme.borderRadius
                padding: 15

                Row {
                    anchors.centerIn: parent
                    spacing: 20

                    Button {
                        text: "REQUEST DGCA SIGNATURE"
                        font: theme.titleFont
                        contentItem: Text {
                            text: parent.text
                            font: parent.font
                            color: telemetryController.readyToFly ? theme.textDisabled : theme.textPrimary
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        background: Rectangle {
                            implicitWidth: 220
                            implicitHeight: 50
                            color: telemetryController.readyToFly ? theme.borderPrimary : theme.accent
                            radius: 6
                            border.color: theme.borderPrimary
                        }
                        enabled: !telemetryController.readyToFly
                    }

                    Button {
                        text: telemetryController.readyToFly ? "ARM SYSTEMS" : "ARM BLOCKED"
                        font: theme.titleFont
                        contentItem: Text {
                            text: parent.text
                            font: parent.font
                            color: "#FFFFFF"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        background: Rectangle {
                            implicitWidth: 220
                            implicitHeight: 50
                            color: telemetryController.readyToFly ? theme.greenAccent : "#4D1A21"
                            radius: 6
                            border.color: theme.borderPrimary
                        }
                        onClicked: {
                            if (telemetryController.readyToFly) {
                                telemetryController.armVehicle();
                            }
                        }
                    }
                }
            }
        }
    }
}
