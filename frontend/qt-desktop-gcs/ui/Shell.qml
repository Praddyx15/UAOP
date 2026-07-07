import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UAOPGCS
import "styles"
import "components"

// Workspace shell (UI_GUIDELINES.md §2): top bar with workspace switcher and
// status strip; dockable-panel host below. Replaces the prototype's
// sidebar/page-stack navigation.
ApplicationWindow {
    id: window
    visible: true
    width: 1440
    height: 860
    title: "UAOP GCS"
    color: theme.bg0

    Theme { id: theme }

    // Floating panel copies (tear-off v0; full move-to-float docking lands M6)
    ListModel { id: floatingPanels }

    header: Rectangle {
        height: 40
        color: theme.bg1

        Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: theme.stroke }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: theme.spacing3
            anchors.rightMargin: theme.spacing3
            spacing: theme.spacing3

            Rectangle {
                width: 22; height: 22; radius: theme.radius
                color: theme.accentColor
                Text { anchors.centerIn: parent; text: "U"; color: theme.bg0; font.bold: true; font.pixelSize: 13 }
            }
            Text { text: "UAOP"; color: theme.fg0; font: theme.headerFont }

            Rectangle { width: 1; Layout.fillHeight: true; Layout.topMargin: 8; Layout.bottomMargin: 8; color: theme.stroke }

            // Workspace switcher — atomic layout switch (UI_GUIDELINES §2)
            Row {
                spacing: theme.spacing1
                Repeater {
                    model: WorkspaceManager.workspaceNames
                    Button {
                        required property string modelData
                        required property int index
                        readonly property bool active: WorkspaceManager.currentIndex === index
                        text: modelData
                        font: theme.panelTitleFont
                        contentItem: Text {
                            text: parent.text
                            font: parent.font
                            color: parent.active ? theme.fg0 : theme.fg1
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        background: Rectangle {
                            implicitHeight: theme.controlHeight
                            implicitWidth: 110
                            radius: theme.radius
                            color: parent.active ? theme.bg2 : "transparent"
                            border.color: parent.active ? theme.accentColor : theme.stroke
                            border.width: 1
                        }
                        onClicked: WorkspaceManager.currentIndex = index
                    }
                }
            }

            Item { Layout.fillWidth: true }

            // Status chips — state honesty, no popups (UX_GUIDELINES §2/§3)
            Row {
                spacing: theme.spacing2

                Rectangle {
                    width: statusText.width + 26; height: theme.controlHeight; radius: theme.radius
                    color: "transparent"
                    border.color: telemetryController.readyToFly ? theme.okGreen : theme.warnRed
                    border.width: 1
                    Rectangle {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left; anchors.leftMargin: 7
                        width: 8; height: 8; radius: 4
                        color: telemetryController.readyToFly ? theme.okGreen : theme.warnRed
                    }
                    Text {
                        id: statusText
                        anchors.centerIn: parent
                        anchors.horizontalCenterOffset: 6
                        text: telemetryController.readyToFly ? "READY" : telemetryController.statusMessage.toUpperCase()
                        color: telemetryController.readyToFly ? theme.okGreen : theme.warnRed
                        font: theme.panelTitleFont
                    }
                }

                Rectangle {
                    width: 52; height: theme.controlHeight; radius: theme.radius
                    color: "transparent"
                    border.color: telemetryController.e2eeEnabled ? theme.okGreen : theme.fg2
                    border.width: 1
                    Text {
                        anchors.centerIn: parent
                        text: "E2EE"
                        color: telemetryController.e2eeEnabled ? theme.okGreen : theme.fg2
                        font: theme.panelTitleFont
                    }
                }

                Rectangle {
                    width: 52; height: theme.controlHeight; radius: theme.radius
                    color: "transparent"
                    border.color: telemetryController.fhssSyncActive ? theme.okGreen : theme.fg2
                    border.width: 1
                    Text {
                        anchors.centerIn: parent
                        text: "FHSS"
                        color: telemetryController.fhssSyncActive ? theme.okGreen : theme.fg2
                        font: theme.panelTitleFont
                    }
                }
            }
        }
    }

    PanelHost {
        anchors.fill: parent
        anchors.margins: theme.spacing1
        onFloatRequested: panelId => floatingPanels.append({ "panelId": panelId })
    }

    Instantiator {
        model: floatingPanels
        delegate: Window {
            required property string panelId
            required property int index
            width: 640
            height: 480
            visible: true
            title: "UAOP — " + panelId
            color: theme.bg0
            onClosing: floatingPanels.remove(index)

            PanelChrome {
                anchors.fill: parent
                anchors.margins: theme.spacing1
                panelId: parent.panelId
                isFloating: true
            }
        }
    }
}
