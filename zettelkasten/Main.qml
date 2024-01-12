// Copyright (C) 2024 Shawn Rutledge <s@ecloud.org>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Shapes
import Qt.labs.folderlistmodel
import org.nettebook.zettelkasten

Window {
    id: surfaceWindow
    width: 3840; height: 2160
    visibility: Window.Maximized
    flags: Qt.CustomizeWindowHint | Qt.FramelessWindowHint
    color: "#00000000"
    title: qsTr("Nettebook Zettelkasten")

    Image {
        property int highestZ: 0

        id: surface
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        width: 100000; height: 100000
        source: "resources/gridsquare.png"
        fillMode: Image.Tile

        ZkModel {
            id: folderModel
            folder: "file:example"
            documentProvider: (index) => { return peter.itemAt(index)?.edit?.textDocument; }
            watcherEnabled: false // UI is too crash-prone for now
        }

        Repeater {
            id: peter
            model: folderModel

            function saveAndReload() {
                for (var i = 0; i < peter.count; ++i)
                    peter.itemAt(i).save()
                peter.model = null
                peter.model = folderModel
            }

            function reload() {
                peter.model = null
                peter.model = folderModel
            }

            delegate: MainDelegate { }

        } // Repeater

        WheelHandler {
            property: "scale"
        }

        PinchHandler {
            minimumRotation: 0
            maximumRotation: 1
        }
    }

    Rectangle {
        border.color: "tomato"
        border.width: 2
        radius: 6
        color: "#CCFFEEDD"
        width: toastMessage.implicitWidth + 20
        height: toastMessage.implicitHeight + 20
        visible: toastMessage.text !== ""
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 10

        Timer {
            id: hideErrorsTimer
            interval: 5000
            onTriggered: toastMessage.text = ""
        }

        Text {
            id: toastMessage
            anchors.centerIn: parent
            onTextChanged: hideErrorsTimer.start()
        }
    }

    QuadPieMenu {
        id: pieMenu
        labels: [ "", "🔗", "🗑️", "" ]
        onTriggered:
            (text) => {
                const p = surface.mapFromItem(pieMenu.parent, pieMenu.point.position)
                const ctxItem = surface.childAt(p.x, p.y)
                // surfaceWindow.contentItem.dumpItemTree()
                console.log(text, "context menu on", ctxItem)
                if (text === "🔗")
                    ctxItem.addPendingLink()
                else if (text === "🗑️")
                    ctxItem.remove()
            }
    }

    Shortcut {
        sequence: StandardKey.Quit
        onActivated: {
            for (var i = 0; i < peter.count; ++i)
                peter.itemAt(i).save()
            Qt.quit()
        }
    }

    Shortcut {
        sequence: StandardKey.New
        onActivated: {
            folderModel.makeNew()
            peter.saveAndReload()
        }
    }

    Shortcut {
        sequence: StandardKey.Save
        onActivated: peter.saveAndReload()
    }

    Shortcut {
        sequence: "Ctrl+Alt+A" // Arrange
        onActivated: {
            for (var i = 0; i < peter.count; ++i)
                peter.itemAt(i).setRandomPosition()
        }
    }
}
