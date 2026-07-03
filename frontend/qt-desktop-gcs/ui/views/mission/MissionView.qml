import QtQuick
import QtQuick.Controls
import UAOPGCS

Item {
    id: root

    Theme { id: theme }

    // Mock waypoint coordinates (scaled relative offsets for the Canvas radar)
    property var waypoints: [
        { x: 0, y: 0, label: "HOME" },
        { x: -50, y: -80, label: "WP 1" },
        { x: 70, y: -120, label: "WP 2" },
        { x: 110, y: 20, label: "WP 3" },
        { x: 30, y: 90, label: "WP 4" }
    ]

    property int activeWaypointIndex: 2
    property real geofenceRadius: 160

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
                text: "TACTICAL RADAR & MISSION PLANNER"
                color: theme.textPrimary
                font: theme.headerFont
            }

            Grid {
                width: parent.width
                columns: parent.width > 900 ? 2 : 1
                spacing: 20

                // Card 1: Canvas Radar Map
                Rectangle {
                    width: parent.width > 900 ? (parent.width - 20) / 2 : parent.width
                    height: 400
                    color: theme.panelBg
                    border.color: theme.borderPrimary
                    radius: theme.borderRadius
                    clip: true

                    Text {
                        text: "TACTICAL RADAR TRACK"
                        color: theme.textSecondary
                        font: theme.titleFont
                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.margins: 15
                    }

                    Canvas {
                        id: radarCanvas
                        anchors.centerIn: parent
                        width: 320
                        height: 320

                        Connections {
                            target: telemetryController
                            function onTelemetryChanged() {
                                radarCanvas.requestPaint();
                            }
                        }

                        onPaint: {
                            var ctx = getContext("2d");
                            ctx.reset();

                            var cx = width / 2;
                            var cy = height / 2;

                            // 1. Draw Radar grid lines
                            ctx.strokeStyle = "#1A323E";
                            ctx.lineWidth = 1;
                            
                            // Concentric circles
                            for (var r = 40; r <= 160; r += 40) {
                                ctx.beginPath();
                                ctx.arc(cx, cy, r, 0, 2 * Math.PI);
                                ctx.stroke();
                            }

                            // Crosshair lines
                            ctx.beginPath();
                            ctx.moveTo(cx - 160, cy);
                            ctx.lineTo(cx + 160, cy);
                            ctx.moveTo(cx, cy - 160);
                            ctx.lineTo(cx, cy + 160);
                            ctx.stroke();

                            // 2. Draw Geofence Boundary (Aviation fence)
                            ctx.strokeStyle = telemetryController.signalJamming > 70 ? theme.redAccent : theme.orangeAccent;
                            ctx.lineWidth = 2;
                            ctx.setLineDash([5, 5]);
                            ctx.beginPath();
                            ctx.arc(cx, cy, root.geofenceRadius, 0, 2 * Math.PI);
                            ctx.stroke();
                            ctx.setLineDash([]); // Reset dash

                            // 3. Draw Waypoint Mission Path
                            ctx.strokeStyle = theme.accent;
                            ctx.lineWidth = 2;
                            ctx.beginPath();
                            for (var i = 0; i < root.waypoints.length; i++) {
                                var wp = root.waypoints[i];
                                var px = cx + wp.x;
                                var py = cy + wp.y;
                                if (i === 0) {
                                    ctx.moveTo(px, py);
                                } else {
                                    ctx.lineTo(px, py);
                                }
                            }
                            ctx.closePath();
                            ctx.stroke();

                            // 4. Draw Waypoints nodes
                            for (var j = 0; j < root.waypoints.length; j++) {
                                var node = root.waypoints[j];
                                var nx = cx + node.x;
                                var ny = cy + node.y;

                                ctx.fillStyle = (j === root.activeWaypointIndex) ? theme.greenAccent : theme.accent;
                                ctx.beginPath();
                                ctx.arc(nx, ny, 6, 0, 2 * Math.PI);
                                ctx.fill();

                                ctx.fillStyle = theme.textPrimary;
                                ctx.font = "10px Inter";
                                ctx.fillText(node.label, nx + 10, ny - 6);
                            }

                            // 5. Draw Drone Live Position (derived relative to Home WP 0)
                            // We mock position offset based on simulated attitude/heading
                            var droneOffsetAngle = (telemetryController.yaw * Math.PI / 180.0);
                            var droneDistance = 65; // Simulated radius distance
                            var dx = cx + Math.sin(droneOffsetAngle) * droneDistance;
                            var dy = cy - Math.cos(droneOffsetAngle) * droneDistance;

                            // Draw trail/vector
                            ctx.strokeStyle = theme.greenAccent;
                            ctx.lineWidth = 1;
                            ctx.beginPath();
                            ctx.moveTo(cx, cy);
                            ctx.lineTo(dx, dy);
                            ctx.stroke();

                            // Draw drone icon (triangle pointing to Yaw)
                            ctx.fillStyle = theme.greenAccent;
                            ctx.translate(dx, dy);
                            ctx.rotate(droneOffsetAngle);
                            ctx.beginPath();
                            ctx.moveTo(0, -10);
                            ctx.lineTo(-7, 8);
                            ctx.lineTo(7, 8);
                            ctx.closePath();
                            ctx.fill();
                            ctx.resetTransform();
                        }
                    }
                }

                // Card 2: Mission Telemetry & Waypoint Details
                Rectangle {
                    width: parent.width > 900 ? (parent.width - 20) / 2 : parent.width
                    height: 400
                    color: theme.panelBg
                    border.color: theme.borderPrimary
                    radius: theme.borderRadius
                    padding: 20

                    Column {
                        anchors.fill: parent
                        spacing: 15

                        Text {
                            text: "MISSION ROUTE EXECUTION"
                            color: theme.textSecondary
                            font: theme.titleFont
                        }

                        Row {
                            spacing: 12
                            Text { text: "Active Waypoint: "; color: theme.textSecondary; font: theme.bodyFont; width: 150 }
                            Text { 
                                text: "WAYPOINT " + root.activeWaypointIndex
                                color: theme.greenAccent
                                font: theme.titleFont 
                            }
                        }

                        Row {
                            spacing: 12
                            Text { text: "Target Altitude: "; color: theme.textSecondary; font: theme.bodyFont; width: 150 }
                            Text { text: "60.0 m (AGL)"; color: theme.textPrimary; font: theme.telemetryFont }
                        }

                        Row {
                            spacing: 12
                            Text { text: "Distance to Target: "; color: theme.textSecondary; font: theme.bodyFont; width: 150 }
                            Text { text: "124.5 m"; color: theme.textPrimary; font: theme.telemetryFont }
                        }

                        Row {
                            spacing: 12
                            Text { text: "ETA to Destination: "; color: theme.textSecondary; font: theme.bodyFont; width: 150 }
                            Text { text: "00:04:12"; color: theme.accent; font: theme.telemetryFont }
                        }

                        Row {
                            spacing: 12
                            Text { text: "Geofence Status: "; color: theme.textSecondary; font: theme.bodyFont; width: 150 }
                            Text { 
                                text: telemetryController.signalJamming > 70 ? "WARNING: RTL TRIGGER NEAR LIMIT" : "OK (INSIDE ENVELOPE)"
                                color: telemetryController.signalJamming > 70 ? theme.redAccent : theme.greenAccent
                                font: theme.bodyFont 
                            }
                        }

                        Rectangle {
                            width: parent.width
                            height: 1
                            color: theme.borderPrimary
                        }

                        Text {
                            text: "TACTICAL FLIGHT ENVELOPE CONTROL"
                            color: theme.textSecondary
                            font: theme.titleFont
                        }

                        Row {
                            spacing: 15
                            anchors.horizontalCenter: parent.horizontalCenter

                            Button {
                                text: "ABORT MISSION (RTL)"
                                font: theme.bodyFont
                                contentItem: Text {
                                    text: parent.text
                                    font: parent.font
                                    color: "#FFFFFF"
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                                background: Rectangle {
                                    implicitWidth: 160
                                    implicitHeight: 40
                                    color: theme.orangeAccent
                                    radius: 6
                                }
                                onClicked: {
                                    telemetryController.abortMission();
                                }
                            }

                            Button {
                                text: "EMERGENCY CUT CORES"
                                font: theme.bodyFont
                                contentItem: Text {
                                    text: parent.text
                                    font: parent.font
                                    color: "#FFFFFF"
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                                background: Rectangle {
                                    implicitWidth: 160
                                    implicitHeight: 40
                                    color: theme.redAccent
                                    radius: 6
                                }
                                onClicked: {
                                    telemetryController.abortMission();
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
