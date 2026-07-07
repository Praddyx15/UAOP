import QtQuick

// Design tokens per docs/04_UI_Architecture/STYLE_GUIDE.md. Instantiable for now
// (legacy views do `Theme { id: theme }`); conversion to a true singleton happens
// with the M2/M6 re-theme. Legacy aliases at the bottom keep the migrated
// prototype views compiling while mapping them onto the aerospace palette.
QtObject {
    // ── Base surfaces (STYLE_GUIDE §2) ──────────────────────────────────────
    readonly property color bg0: "#0B0E11"          // window ground
    readonly property color bg1: "#12161B"          // panel surfaces
    readonly property color bg2: "#1A2027"          // cards, wells, inputs
    readonly property color stroke: "#2A323C"       // borders, dividers, grid
    readonly property color fg0: "#E6EAEE"          // primary text/values
    readonly property color fg1: "#9AA5B1"          // labels, units
    readonly property color fg2: "#5C6773"          // disabled, hints
    readonly property color accentColor: "#3B82C4"  // focus/selection only

    // ── Semantic (aviation-standard meanings — never repurposed) ────────────
    readonly property color warnRed: "#E5484D"
    readonly property color cautionAmber: "#F5A524"
    readonly property color advisoryCyan: "#4CC2D9"
    readonly property color okGreen: "#3DBB7C"
    readonly property color simViolet: "#8B7EC8"    // sim/replay watermark, exclusive

    // ── Geometry (STYLE_GUIDE §4) ───────────────────────────────────────────
    readonly property int spacing1: 4
    readonly property int spacing2: 8
    readonly property int spacing3: 12
    readonly property int spacing4: 16
    readonly property int radius: 3
    readonly property int controlHeight: 26
    readonly property int panelHeaderHeight: 26

    // ── Typography (STYLE_GUIDE §3; system fallbacks until fonts are bundled)
    readonly property string uiFontFamily: Qt.application.font.family
    readonly property string monoFontFamily: "Consolas"

    readonly property font panelTitleFont: Qt.font({ family: uiFontFamily, pixelSize: 11, bold: true, letterSpacing: 1.2, capitalization: Font.AllUppercase })
    readonly property font labelFont: Qt.font({ family: uiFontFamily, pixelSize: 11 })
    readonly property font bodyFont2: Qt.font({ family: uiFontFamily, pixelSize: 12 })
    readonly property font valueFont: Qt.font({ family: monoFontFamily, pixelSize: 14, bold: true })
    readonly property font hudValueFont: Qt.font({ family: monoFontFamily, pixelSize: 22, bold: true })

    // ── Legacy aliases (prototype views; remove at M2/M6 re-theme) ──────────
    readonly property color background: bg0
    readonly property color panelBg: bg1
    readonly property color borderPrimary: stroke
    readonly property color textPrimary: fg0
    readonly property color textSecondary: fg1
    readonly property color textDisabled: fg2
    readonly property color accent: accentColor
    readonly property color greenAccent: okGreen
    readonly property color redAccent: warnRed
    readonly property color orangeAccent: cautionAmber
    readonly property int borderRadius: radius
    readonly property real opacityGlass: 1.0        // glass effects retired (STYLE_GUIDE §1)
    readonly property font headerFont: Qt.font({ family: uiFontFamily, pixelSize: 16, bold: true })
    readonly property font titleFont: panelTitleFont
    readonly property font bodyFont: bodyFont2
    readonly property font telemetryFont: valueFont
}
