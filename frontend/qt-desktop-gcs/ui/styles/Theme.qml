pragma Singleton
import QtQuick

QtObject {
    // Brand Color Palette (Sleek Dark Cyberpunk theme)
    readonly property color background: "#0F111A"
    readonly property color panelBg: "#171A26"
    readonly property color borderPrimary: "#262B3D"
    readonly property color textPrimary: "#F0F2F7"
    readonly property color textSecondary: "#8A94A6"
    readonly property color textDisabled: "#4D5666"

    // Functional State Colors
    readonly property color accent: "#00E5FF"        // Electric Cyan
    readonly property color greenAccent: "#00E676"   // Tactical Active Green
    readonly property color redAccent: "#FF1744"     // Warning Safety Crimson
    readonly property color orangeAccent: "#FF9100"  // Warning Alert Orange

    // Visual Constants
    readonly property int borderRadius: 12
    readonly property real opacityGlass: 0.85

    // Typography
    readonly property font headerFont: Qt.font({
        family: "Outfit",
        pixelSize: 22,
        bold: true
    })

    readonly property font titleFont: Qt.font({
        family: "Inter",
        pixelSize: 16,
        bold: true
    })

    readonly property font bodyFont: Qt.font({
        family: "Inter",
        pixelSize: 13,
        bold: false
    })

    readonly property font telemetryFont: Qt.font({
        family: "Courier New",
        pixelSize: 14,
        bold: true
    })
}
