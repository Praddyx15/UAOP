import QtQuick
import QtQuick.Controls
import UAOPGCS

Button {
    id: control
    width: parent.width
    height: 45

    property bool active: false

    Theme { id: theme }

    contentItem: Text {
        text: control.text
        font: theme.titleFont
        color: control.active ? theme.accent : theme.textSecondary
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        leftPadding: 15
    }

    background: Rectangle {
        color: control.active ? "#1A263D" : "transparent"
        border.color: control.active ? theme.accent : "transparent"
        border.width: control.active ? 1 : 0
        radius: 6

        // Indicator bar on the left
        Rectangle {
            width: 4
            height: parent.height - 16
            color: theme.accent
            radius: 2
            anchors.left: parent.left
            anchors.leftMargin: 4
            anchors.verticalCenter: parent.verticalCenter
            visible: control.active
        }
    }
}
