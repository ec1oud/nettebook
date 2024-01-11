import QtQuick

DropArea {
    id: root
    property string text
    property url linkUrl
    property TextEdit textEdit: null

    width: Math.max(icon.implicitWidth, 16)
    height: Math.max(icon.implicitHeight, 16)

    onEntered: (ev) => { console.log("entered", ev, ev.text, ev.urls, "from", root.drag.source) }
    onDropped:
        (ev) => {
            console.log("dropped", ev, ev.text, ev.urls, "from", root.drag.source, root.drag.source.textDocument,
                        "onto", root.linkUrl, "in", root.textEdit.textDocument)

            Utils.insertLink(root.drag.source.textDocument, root.linkUrl,
                             root.textEdit.textDocument, ev.urls[0])
        }

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
        Drag.source: root.textEdit
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
