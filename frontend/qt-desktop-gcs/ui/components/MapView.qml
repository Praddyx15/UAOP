import QtQuick
import "../styles"

// Interim raster map view (ADR-0020): pans/zooms a grid of MBTiles tiles
// served by MbtilesProvider via image://mbtiles/z/x/y. Deliberately minimal —
// this is the M0.7 spike output, not the full M2.7 map panel (vehicle
// overlay, mission layer, geofences arrive there).
Item {
    id: root
    clip: true

    property real centerLon: 77.0266  // Gurugram — HARDWARE_ARCHITECTURE.md sim default
    property real centerLat: 28.4595
    property int zoom: 2
    readonly property int tileSize: 256

    Theme { id: theme }

    function tileCount() { return Math.pow(2, zoom) }

    // Fractional tile coords of the viewport center at the current zoom.
    QtObject {
        id: center
        property real fx: 0
        property real fy: 0
    }

    onCenterLonChanged: recomputeCenter()
    onCenterLatChanged: recomputeCenter()
    onZoomChanged: recomputeCenter()
    Component.onCompleted: recomputeCenter()

    function recomputeCenter() {
        const n = tileCount()
        const latRad = centerLat * Math.PI / 180
        center.fx = (centerLon + 180) / 360 * n
        center.fy = (1 - Math.asinh(Math.tan(latRad)) / Math.PI) / 2 * n
    }

    Rectangle { anchors.fill: parent; color: theme.bg0 }

    Item {
        id: tileField
        anchors.fill: parent

        readonly property int halfCols: Math.ceil(root.width / tileSize / 2) + 1
        readonly property int halfRows: Math.ceil(root.height / tileSize / 2) + 1
        readonly property int centerTileX: Math.floor(center.fx)
        readonly property int centerTileY: Math.floor(center.fy)
        readonly property real offsetX: (center.fx - centerTileX) * tileSize
        readonly property real offsetY: (center.fy - centerTileY) * tileSize

        Repeater {
            model: (tileField.halfCols * 2 + 1) * (tileField.halfRows * 2 + 1)

            Image {
                required property int index
                readonly property int col: index % (tileField.halfCols * 2 + 1) - tileField.halfCols
                readonly property int row: Math.floor(index / (tileField.halfCols * 2 + 1)) - tileField.halfRows
                readonly property int n: root.tileCount()
                readonly property int tx: ((tileField.centerTileX + col) % n + n) % n
                readonly property int ty: tileField.centerTileY + row

                x: root.width / 2 + col * tileField.tileSize - tileField.offsetX - tileField.tileSize / 2
                y: root.height / 2 + row * tileField.tileSize - tileField.offsetY - tileField.tileSize / 2
                width: tileField.tileSize
                height: tileField.tileSize
                visible: ty >= 0 && ty < n
                source: visible ? ("image://mbtiles/" + root.zoom + "/" + tx + "/" + ty) : ""
                asynchronous: true
                cache: true
                smooth: false // tile edges must stay crisp at integer zoom
            }
        }
    }

    // Center reticle — visual proof the tile math centers correctly (no
    // vehicle overlay yet; that is TELEMETRY_ENGINE-fed, M2.7 scope).
    Rectangle {
        anchors.centerIn: parent
        width: 2; height: 14; color: theme.warnRed
    }
    Rectangle {
        anchors.centerIn: parent
        width: 14; height: 2; color: theme.warnRed
    }

    // Pan (drag) and zoom (wheel) — minimal interaction for the spike.
    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        property real lastX: 0
        property real lastY: 0
        onPressed: (mouse) => { lastX = mouse.x; lastY = mouse.y }
        onPositionChanged: (mouse) => {
            if (!pressed) return
            const dxTiles = (lastX - mouse.x) / tileField.tileSize
            const dyTiles = (lastY - mouse.y) / tileField.tileSize
            let lon = 0, lat = 0
            const n = root.tileCount()
            const newFx = center.fx + dxTiles
            const newFy = center.fy + dyTiles
            lon = newFx / n * 360 - 180
            const latRad = Math.atan(Math.sinh(Math.PI * (1 - 2 * newFy / n)))
            lat = latRad * 180 / Math.PI
            root.centerLon = lon
            root.centerLat = lat
            lastX = mouse.x; lastY = mouse.y
        }
        onWheel: (wheel) => {
            root.zoom = Math.max(0, Math.min(19, root.zoom + (wheel.angleDelta.y > 0 ? 1 : -1)))
        }
    }

    Rectangle {
        anchors.left: parent.left; anchors.bottom: parent.bottom
        anchors.margins: theme.spacing2
        width: zoomLabel.width + theme.spacing2 * 2
        height: theme.controlHeight
        color: theme.bg1
        border.color: theme.stroke
        border.width: 1
        radius: theme.radius
        Text {
            id: zoomLabel
            anchors.centerIn: parent
            text: "Z" + root.zoom
            color: theme.fg1
            font: theme.panelTitleFont
        }
    }
}
