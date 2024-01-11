import QtQuick

DropArea {
    id: root
    property string text
    property url linkUrl

    width: Math.max(icon.implicitWidth, 16)
    height: Math.max(icon.implicitHeight, 16)

    onEntered: (ev) => { console.log("entered", ev, ev?.drag, ev?.text, ev?.urls) }
    onDropped: (ev) => { console.log("dropped", ev, ev?.drag, ev?.text, ev?.urls) }

    Rectangle {
        id: draggable
        Binding on x {
            when: parent == root
            value: 0
        }
        Binding on y {
            when: parent == root
            value: 0
        }
        width: root.width; height: root.width

        // Drag.active: dragHandler.active
        Drag.dragType: Drag.Automatic
        Drag.supportedActions: Qt.CopyAction
        Drag.mimeData: {
            "text/plain": root.text,
            "text/uri-list": [root.linkUrl]
        }
        border.color: Drag.active ? "tomato" : root.containsDrag ? "green" : "black"
        radius: 2

        Text {
            id: icon
            anchors.centerIn: parent
            text: "🔗"
            font.family: "Symbola"
        }

        DragHandler {
            id: dragHandler
            onActiveChanged: {
                if (active) {
                    draggable.Drag.active = true
                } else {
                    var dr = draggable.Drag.drop()
                    console.log("dropped?", dr)
                }
                target.parent = active ? root.parent : root
            }
        }
    }
}
