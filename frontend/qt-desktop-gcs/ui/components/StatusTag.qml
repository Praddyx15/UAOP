import QtQuick
import UAOPGCS

// Status = shape + word + colour, never colour alone (STYLE_GUIDE §4, UX_GUIDELINES §2).
// tone: warning (octagon, red) | caution (triangle, amber) | advisory (circle, cyan) | ok (check, green)
Row {
    id: root

    property string tone: "advisory"
    property string word: ""
    property font wordFont: theme.panelTitleFont

    readonly property color toneColor: tone === "warning" ? theme.warnRedText
                                     : tone === "caution" ? theme.cautionAmber
                                     : tone === "ok" ? theme.okGreen
                                     : theme.advisoryCyan

    spacing: theme.spacing1

    Theme { id: theme }

    Canvas {
        id: icon
        width: 14
        height: 14
        anchors.verticalCenter: parent.verticalCenter
        onPaint: {
            const ctx = getContext("2d")
            ctx.reset()
            ctx.strokeStyle = root.toneColor
            ctx.lineWidth = 1.5
            ctx.beginPath()
            const w = width, h = height
            if (root.tone === "warning") {
                const k = 4
                ctx.moveTo(k, 1); ctx.lineTo(w - k, 1); ctx.lineTo(w - 1, k); ctx.lineTo(w - 1, h - k)
                ctx.lineTo(w - k, h - 1); ctx.lineTo(k, h - 1); ctx.lineTo(1, h - k); ctx.lineTo(1, k)
                ctx.closePath()
            } else if (root.tone === "caution") {
                ctx.moveTo(w / 2, 1); ctx.lineTo(w - 1, h - 1); ctx.lineTo(1, h - 1)
                ctx.closePath()
            } else if (root.tone === "ok") {
                ctx.moveTo(2, h / 2); ctx.lineTo(w / 2 - 1, h - 3); ctx.lineTo(w - 2, 3)
            } else {
                ctx.arc(w / 2, h / 2, w / 2 - 1, 0, 2 * Math.PI)
            }
            ctx.stroke()
        }
        Connections {
            target: root
            function onToneChanged() { icon.requestPaint() }
        }
    }

    Text {
        anchors.verticalCenter: parent.verticalCenter
        text: root.word
        color: root.toneColor
        font: root.wordFont
    }
}
