import QtQuick
import QtQuick.Controls
import UAOPGCS

Item {
    id: root

    Theme { id: theme }

    // Active key rotation timer simulation
    property int keyRotationTimeRemaining: 24
    Timer {
        interval: 1000
        running: true
        repeat: true
        onTriggered: {
            if (root.keyRotationTimeRemaining > 1) {
                root.keyRotationTimeRemaining--;
            } else {
                root.keyRotationTimeRemaining = 30; // reset
            }
        }
    }

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
                text: "TACTICAL COMMUNICATIONS & DEFENCE DIAGNOSTICS"
                color: theme.textPrimary
                font: theme.headerFont
            }

            // Signal Jamming Warning Banner
            Rectangle {
                width: parent.width
                height: telemetryController.signalJamming > 50 ? 60 : 0
                color: "#2E1114"
                border.color: theme.redAccent
                border.width: 1
                radius: 6
                visible: telemetryController.signalJamming > 50
                clip: true

                Behavior on height { NumberAnimation { duration: 250 } }

                Row {
                    anchors.centerIn: parent
                    spacing: 15

                    Text {
                        text: "⚠️ HIGH-POWER RF INTERFERENCE / JAMMING DETECTED"
                        color: theme.redAccent
                        font: theme.titleFont
                        font.bold: true
                    }
                }
            }

            // Top Row: E2EE Encryption details and Jamming Alert
            Grid {
                width: parent.width
                columns: parent.width > 900 ? 2 : 1
                spacing: 20

                // Card 1: End-to-End Encryption Metrics
                Rectangle {
                    width: parent.width > 900 ? (parent.width - 20) / 2 : parent.width
                    height: 280
                    color: theme.panelBg
                    border.color: theme.borderPrimary
                    radius: theme.borderRadius
                    padding: 20

                    Column {
                        anchors.fill: parent
                        spacing: 15

                        Text {
                            text: "AES-256-GCM TELEMETRY ENCRYPTION"
                            color: theme.textSecondary
                            font: theme.titleFont
                        }

                        Row {
                            spacing: 12
                            Text { text: "Link Security Status: "; color: theme.textSecondary; font: theme.bodyFont; width: 140 }
                            Text { 
                                text: telemetryController.e2eeEnabled ? "SECURED & SIGNED" : "UNENCRYPTED LINK"
                                color: telemetryController.e2eeEnabled ? theme.greenAccent : theme.redAccent
                                font: theme.titleFont 
                            }
                        }

                        Row {
                            spacing: 12
                            Text { text: "Encryption Cipher: "; color: theme.textSecondary; font: theme.bodyFont; width: 140 }
                            Text { text: "AES-GCM-256 (Aviation Standard)"; color: theme.textPrimary; font: theme.telemetryFont }
                        }

                        Row {
                            spacing: 12
                            Text { text: "Session Key Rotation: "; color: theme.textSecondary; font: theme.bodyFont; width: 140 }
                            Text { 
                                text: "Every 30s (Next in " + root.keyRotationTimeRemaining + "s)"
                                color: theme.accent
                                font: theme.telemetryFont 
                            }
                        }

                        Row {
                            spacing: 12
                            Text { text: "Packet Authentication: "; color: theme.textSecondary; font: theme.bodyFont; width: 140 }
                            Text { text: "HMAC-SHA-256 (MAVLink V2.0 Signature)"; color: theme.textPrimary; font: theme.telemetryFont }
                        }

                        Row {
                            spacing: 12
                            Text { text: "Cryptographic Seed: "; color: theme.textSecondary; font: theme.bodyFont; width: 140 }
                            Text { 
                                text: telemetryController.e2eeEnabled ? "0xF882A39FE3B1D5C92..." : "NULL"
                                color: theme.textSecondary
                                font: theme.telemetryFont
                                elide: Text.ElideRight
                                width: 220
                            }
                        }
                    }
                }

                // Card 2: RF Spectrum & Anti-Jamming Health
                Rectangle {
                    width: parent.width > 900 ? (parent.width - 20) / 2 : parent.width
                    height: 280
                    color: theme.panelBg
                    border.color: theme.borderPrimary
                    radius: theme.borderRadius
                    padding: 20

                    Column {
                        anchors.fill: parent
                        spacing: 15

                        Text {
                            text: "RF SPECTRUM & ANTI-JAMMING SAFETY"
                            color: theme.textSecondary
                            font: theme.titleFont
                        }

                        Row {
                            spacing: 12
                            Text { text: "Anti-Jamming Mode: "; color: theme.textSecondary; font: theme.bodyFont; width: 150 }
                            Text { 
                                text: telemetryController.fhssSyncActive ? "FHSS DYNAMIC HOPPING" : "STATIC LINK (VULNERABLE)"
                                color: telemetryController.fhssSyncActive ? theme.greenAccent : theme.redAccent
                                font: theme.titleFont 
                            }
                        }

                        Row {
                            spacing: 12
                            Text { text: "Active Channel: "; color: theme.textSecondary; font: theme.bodyFont; width: 150 }
                            Text { 
                                text: "CH " + telemetryController.fhssChannel + " (" + (2400 + telemetryController.fhssChannel * 2.4).toFixed(1) + " MHz)"
                                color: theme.accent
                                font: theme.telemetryFont 
                            }
                        }

                        Row {
                            spacing: 12
                            Text { text: "RF Jamming Indicator: "; color: theme.textSecondary; font: theme.bodyFont; width: 150 }
                            Text { 
                                text: telemetryController.signalJamming + "% Intensity"
                                color: telemetryController.signalJamming > 50 ? theme.redAccent : theme.greenAccent
                                font: theme.telemetryFont 
                            }
                        }

                        Row {
                            spacing: 12
                            Text { text: "Signal-to-Noise Ratio: "; color: theme.textSecondary; font: theme.bodyFont; width: 150 }
                            Text { 
                                text: (45 - (telemetryController.signalJamming * 0.35)).toFixed(1) + " dB"
                                color: telemetryController.signalJamming > 50 ? theme.orangeAccent : theme.greenAccent
                                font: theme.telemetryFont 
                            }
                        }

                        Row {
                            spacing: 12
                            Text { text: "Hardware Failsafe: "; color: theme.textSecondary; font: theme.bodyFont; width: 150 }
                            Text { 
                                text: telemetryController.signalJamming > 70 ? "RTL FAILSAFE ENGAGED" : "AUTO-MITIGATION COMPLIANT"
                                color: telemetryController.signalJamming > 70 ? theme.redAccent : theme.textSecondary
                                font: theme.bodyFont
                            }
                        }
                    }
                }
            }

            // Lower Section: FHSS Channel Grid Display (Military style)
            Rectangle {
                width: parent.width
                height: 250
                color: theme.panelBg
                border.color: theme.borderPrimary
                radius: theme.borderRadius
                padding: 20

                Column {
                    anchors.fill: parent
                    spacing: 15

                    Text {
                        text: "DYNAMIC FHSS FREQUENCY MATRIX (2.4 GHz ISM)"
                        color: theme.textSecondary
                        font: theme.titleFont
                    }

                    // Grid of 30 channels representing frequency hopping
                    Grid {
                        width: parent.width
                        columns: 10
                        spacing: 8

                        Repeater {
                            model: 30
                            delegate: Rectangle {
                                width: (parent.width - 72) / 10
                                height: 35
                                color: {
                                    if (telemetryController.fhssChannel === index) {
                                        return telemetryController.signalJamming > 50 ? "#4A1A21" : "#112C24";
                                    }
                                    return theme.background;
                                }
                                border.color: {
                                    if (telemetryController.fhssChannel === index) {
                                        return telemetryController.signalJamming > 50 ? theme.redAccent : theme.greenAccent;
                                    }
                                    return theme.borderPrimary;
                                }
                                border.width: 1
                                radius: 4

                                Text {
                                    text: "CH " + (index + 1)
                                    color: {
                                        if (telemetryController.fhssChannel === index) {
                                            return telemetryController.signalJamming > 50 ? theme.redAccent : theme.greenAccent;
                                        }
                                        return theme.textSecondary;
                                    }
                                    font.pixelSize: 10
                                    font.family: "Courier New"
                                    font.bold: true
                                    anchors.centerIn: parent
                                }
                            }
                        }
                    }

                    Row {
                        spacing: 20
                        anchors.horizontalCenter: parent.horizontalCenter

                        Row {
                            spacing: 8
                            Rectangle { width: 12; height: 12; color: theme.background; border.color: theme.borderPrimary; radius: 2; anchors.verticalCenter: parent.verticalCenter }
                            Text { text: "Channel Idle"; color: theme.textSecondary; font: theme.bodyFont }
                        }
                        Row {
                            spacing: 8
                            Rectangle { width: 12; height: 12; color: "#112C24"; border.color: theme.greenAccent; radius: 2; anchors.verticalCenter: parent.verticalCenter }
                            Text { text: "Active Hopping Sync"; color: theme.textSecondary; font: theme.bodyFont }
                        }
                        Row {
                            spacing: 8
                            Rectangle { width: 12; height: 12; color: "#4A1A21"; border.color: theme.redAccent; radius: 2; anchors.verticalCenter: parent.verticalCenter }
                            Text { text: "Active Jamming Interference"; color: theme.textSecondary; font: theme.bodyFont }
                        }
                    }
                }
            }
        }
    }
}
