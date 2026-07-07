import QtQuick
import QtQuick.Controls
import UAOPGCS
import "../styles"

// Panel chrome per UI_GUIDELINES.md §6: header with uppercase title + float
// affordance, content loaded from the registry source. Command-class controls
// never live in the chrome (UI_GUIDELINES §7).
Rectangle {
    id: root

    required property string panelId
    property bool isFloating: false

    signal floatRequested(string panelId)

    readonly property var def: PanelRegistry.panel(panelId)
    readonly property bool known: def && def.source !== undefined

    color: theme.bg1
    border.color: theme.stroke
    border.width: 1
    radius: theme.radius
    implicitWidth: known ? def.minWidth : 320
    implicitHeight: known ? def.minHeight : 240

    Theme { id: theme }

    Column {
        anchors.fill: parent
        anchors.margins: 1

        Rectangle {
            id: header
            width: parent.width
            height: theme.panelHeaderHeight
            color: theme.bg2

            Text {
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: theme.spacing2
                text: root.known ? root.def.title : "UNKNOWN PANEL"
                color: theme.fg1
                font: theme.panelTitleFont
            }

            ToolButton {
                visible: !root.isFloating
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                width: theme.panelHeaderHeight
                height: theme.panelHeaderHeight
                contentItem: Text {
                    text: "↗"
                    color: theme.fg1
                    font.pixelSize: 12
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle { color: parent.hovered ? theme.stroke : "transparent" }
                onClicked: root.floatRequested(root.panelId)

                ToolTip.visible: hovered
                ToolTip.text: "Open floating copy"
                ToolTip.delay: 600
            }
        }

        Loader {
            width: parent.width
            height: parent.height - header.height
            source: root.known ? Qt.resolvedUrl("../" + root.def.source) : ""

            // Absence is explicit, never a blank region (UX_GUIDELINES §3)
            Text {
                anchors.centerIn: parent
                visible: !root.known
                text: "Panel '" + root.panelId + "' is not registered"
                color: theme.cautionAmber
                font: theme.bodyFont2
            }
        }
    }
}
