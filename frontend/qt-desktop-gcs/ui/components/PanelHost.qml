import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import UAOPGCS
import "../styles"

// Renders the active workspace: resizable columns (SplitView), each column a
// stack of panels (tabbed when more than one). Layout comes from
// WorkspaceManager.currentLayout; switching workspaces rebuilds atomically.
Item {
    id: root

    signal floatRequested(string panelId)

    Theme { id: theme }

    SplitView {
        anchors.fill: parent
        orientation: Qt.Horizontal

        handle: Rectangle {
            implicitWidth: 5
            color: SplitHandle.pressed ? theme.accentColor : theme.bg0
            Rectangle {
                anchors.centerIn: parent
                width: 1
                height: parent.height
                color: theme.stroke
            }
        }

        Repeater {
            model: WorkspaceManager.currentLayout

            delegate: Item {
                id: column

                required property var modelData

                readonly property var panelIds: modelData.panels
                readonly property bool tabbed: panelIds.length > 1

                SplitView.fillHeight: true
                SplitView.preferredWidth: root.width * modelData.stretch /
                                          totalStretch()
                SplitView.minimumWidth: 240

                function totalStretch() {
                    let sum = 0
                    const cols = WorkspaceManager.currentLayout
                    for (let i = 0; i < cols.length; ++i)
                        sum += cols[i].stretch
                    return Math.max(sum, 1)
                }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: theme.spacing1
                    spacing: theme.spacing1

                    TabBar {
                        id: columnTabs
                        visible: column.tabbed
                        Layout.fillWidth: true
                        background: Rectangle { color: theme.bg0 }

                        Repeater {
                            model: column.panelIds
                            TabButton {
                                required property string modelData
                                text: {
                                    const def = PanelRegistry.panel(modelData)
                                    return def.title !== undefined ? def.title : modelData
                                }
                                font: theme.panelTitleFont
                                contentItem: Text {
                                    text: parent.text
                                    font: parent.font
                                    color: parent.checked ? theme.fg0 : theme.fg1
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                                background: Rectangle {
                                    color: parent.checked ? theme.bg2 : theme.bg1
                                    border.color: theme.stroke
                                    border.width: 1
                                }
                            }
                        }
                    }

                    StackLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        currentIndex: column.tabbed ? columnTabs.currentIndex : 0

                        Repeater {
                            model: column.panelIds
                            PanelChrome {
                                required property string modelData
                                panelId: modelData
                                onFloatRequested: id => root.floatRequested(id)
                            }
                        }
                    }
                }
            }
        }
    }
}
