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
                text: "FLIGHT TELEMETRY (LIVE)"
                color: theme.textPrimary
                font: theme.headerFont
            }

            // Central grid containing Attitude Indicator and Textual Telemetry
            Grid {
                width: parent.width
                columns: parent.width > 900 ? 2 : 1
                spacing: 20

                // Card 1: Virtual Attitude Indicator
                Rectangle {
                    width: parent.width > 900 ? (parent.width - 20) / 2 : parent.width
                    height: 320
                    color: theme.panelBg
                    border.color: theme.borderPrimary
                    radius: theme.borderRadius
                    clip: true

                    Text {
                        text: "ATTITUDE HORIZON"
                        color: theme.textSecondary
                        font: theme.titleFont
                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.margins: 15
                    }

                    // Canvas-based artificial horizon
                    Canvas {
                        id: attitudeCanvas
                        anchors.centerIn: parent
                        width: 200
                        height: 200

                        Connections {
                            target: telemetryController
                            function onTelemetryChanged() {
                                attitudeCanvas.requestPaint();
                            }
                        }

                        onPaint: {
                            var ctx = getContext("2d");
                            ctx.reset();

                            var cx = width / 2;
                            var cy = height / 2;
                            var r = width / 2 - 10;

                            // Draw attitude indicator background clip path
                            ctx.beginPath();
                            ctx.arc(cx, cy, r, 0, 2 * Math.PI);
                            ctx.clip();

                            // Calculate pitch offset and roll rotation
                            var pitchOffset = telemetryController.pitch * 3.0; // scaling pitch to pixels
                            var rollRad = telemetryController.roll * Math.PI / 180.0;

                            // Rotate and translate for pitch/roll
                            ctx.translate(cx, cy);
                            ctx.rotate(-rollRad);
                            ctx.translate(-cx, -cy);

                            // Draw Sky (Blue)
                            ctx.fillStyle = "#1E88E5";
                            ctx.fillRect(-width, -height, width * 3, height * 3);

                            // Draw Ground (Brown)
                            ctx.fillStyle = "#5D4037";
                            ctx.beginPath();
                            ctx.moveTo(-width, cy + pitchOffset);
                            ctx.lineTo(width * 2, cy + pitchOffset);
                            ctx.lineTo(width * 2, height * 2);
                            ctx.lineTo(-width, height * 2);
                            ctx.closePath();
                            ctx.fill();

                            // Horizon Line
                            ctx.strokeStyle = "#FFFFFF";
                            ctx.lineWidth = 3;
                            ctx.beginPath();
                            ctx.moveTo(-width, cy + pitchOffset);
                            ctx.lineTo(width * 2, cy + pitchOffset);
                            ctx.stroke();

                            // Reset transformations
                            ctx.resetTransform();

                            // Static Plane Indicator overlay
                            ctx.strokeStyle = theme.accent;
                            ctx.lineWidth = 4;
                            ctx.beginPath();
                            // Left wing
                            ctx.moveTo(cx - 50, cy);
                            ctx.lineTo(cx - 20, cy);
                            ctx.lineTo(cx - 10, cy + 10);
                            // Right wing
                            ctx.moveTo(cx + 50, cy);
                            ctx.lineTo(cx + 20, cy);
                            ctx.lineTo(cx + 10, cy + 10);
                            // Center dot
                            ctx.arc(cx, cy, 3, 0, 2 * Math.PI);
                            ctx.stroke();

                            // Outer Ring
                            ctx.strokeStyle = theme.borderPrimary;
                            ctx.lineWidth = 6;
                            ctx.beginPath();
                            ctx.arc(cx, cy, r, 0, 2 * Math.PI);
                            ctx.stroke();
                        }
                    }

                    // Attitude textual overlay
                    Row {
                        anchors.bottom: parent.bottom
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.margins: 15
                        spacing: 20

                        Text {
                            text: "ROLL: " + telemetryController.roll.toFixed(1) + "°"
                            color: theme.textPrimary
                            font: theme.telemetryFont
                        }
                        Text {
                            text: "PITCH: " + telemetryController.pitch.toFixed(1) + "°"
                            color: theme.textPrimary
                            font: theme.telemetryFont
                        }
                        Text {
                            text: "YAW: " + telemetryController.yaw.toFixed(0) + "°"
                            color: theme.textPrimary
                            font: theme.telemetryFont
                        }
                    }
                }

                // Card 2: GPS & Link Metrics
                Rectangle {
                    width: parent.width > 900 ? (parent.width - 20) / 2 : parent.width
                    height: 320
                    color: theme.panelBg
                    border.color: theme.borderPrimary
                    radius: theme.borderRadius
                    padding: 20

                    Text {
                        text: "POSITION & NAVIGATION STATE"
                        color: theme.textSecondary
                        font: theme.titleFont
                        anchors.left: parent.left
                        anchors.top: parent.top
                    }

                    Column {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.topMargin: 40
                        spacing: 12

                        Row {
                            spacing: 10
                            Text { text: "Latitude: "; color: theme.textSecondary; font: theme.bodyFont; width: 100 }
                            Text { text: telemetryController.latitude.toFixed(6); color: theme.textPrimary; font: theme.telemetryFont }
                        }
                        Row {
                            spacing: 10
                            Text { text: "Longitude: "; color: theme.textSecondary; font: theme.bodyFont; width: 100 }
                            Text { text: telemetryController.longitude.toFixed(6); color: theme.textPrimary; font: theme.telemetryFont }
                        }
                        Row {
                            spacing: 10
                            Text { text: "Altitude MSL: "; color: theme.textSecondary; font: theme.bodyFont; width: 100 }
                            Text { text: telemetryController.altitudeWGS84.toFixed(1) + " m"; color: theme.textPrimary; font: theme.telemetryFont }
                        }
                        Row {
                            spacing: 10
                            Text { text: "Altitude AGL: "; color: theme.textSecondary; font: theme.bodyFont; width: 100 }
                            Text { text: telemetryController.altitudeAGL.toFixed(1) + " m"; color: theme.textPrimary; font: theme.telemetryFont }
                        }
                        Row {
                            spacing: 10
                            Text { text: "Ground Speed: "; color: theme.textSecondary; font: theme.bodyFont; width: 100 }
                            Text { text: telemetryController.groundSpeed.toFixed(1) + " m/s"; color: theme.textPrimary; font: theme.telemetryFont }
                        }
                        Row {
                            spacing: 10
                            Text { text: "GPS Satellites: "; color: theme.textSecondary; font: theme.bodyFont; width: 100 }
                            Text { 
                                text: telemetryController.satellites.toString()
                                color: telemetryController.satellites >= 6 ? theme.greenAccent : theme.redAccent
                                font: theme.telemetryFont 
                            }
                        }
                    }
                }
            }

            // Row 2: Diagnostics & System Metrics
            Rectangle {
                width: parent.width
                height: 120
                color: theme.panelBg
                border.color: theme.borderPrimary
                radius: theme.borderRadius
                padding: 20

                Row {
                    anchors.fill: parent
                    spacing: 40

                    Column {
                        spacing: 5
                        Text { text: "BATTERY VOLTAGE"; color: theme.textSecondary; font: theme.bodyFont }
                        Text { 
                            text: telemetryController.batteryVoltage.toFixed(2) + " V"
                            color: telemetryController.batteryRemaining > 0.20 ? theme.textPrimary : theme.redAccent
                            font: theme.headerFont 
                        }
                    }

                    Column {
                        spacing: 5
                        Text { text: "BATTERY CHARGE"; color: theme.textSecondary; font: theme.bodyFont }
                        ProgressBar {
                            value: telemetryController.batteryRemaining
                            width: 150
                            background: Rectangle {
                                implicitHeight: 8
                                color: theme.borderPrimary
                                radius: 4
                            }
                            contentItem: Item {
                                implicitHeight: 8
                                Rectangle {
                                    width: parent.parent.value * parent.parent.width
                                    height: parent.parent.height
                                    radius: 4
                                    color: telemetryController.batteryRemaining > 0.20 ? theme.greenAccent : theme.redAccent
                                }
                            }
                        }
                        Text { 
                            text: (telemetryController.batteryRemaining * 100).toFixed(0) + " %"
                            color: telemetryController.batteryRemaining > 0.20 ? theme.textSecondary : theme.redAccent
                            font: theme.bodyFont 
                        }
                    }

                    Column {
                        spacing: 5
                        Text { text: "CPU WORKLOAD"; color: theme.textSecondary; font: theme.bodyFont }
                        Text { text: telemetryController.cpuLoad.toFixed(1) + "%"; color: theme.accent; font: theme.headerFont }
                    }
                }
            }
        }
    }
}
