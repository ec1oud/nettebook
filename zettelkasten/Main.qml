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

            delegate: Rectangle {
                required property int index
                required property date fileModified
                required property string fileName
                required property url fileUrl
                required property string title
                property alias edit: edit
                property alias yaml: yaml
                property point position: Qt.point(x, y)
                id: notepage
                objectName: "frame: " + fileName
                color: "lightyellow"
                width: Math.max(360, title.implicitWidth); height: 240
                x: yaml.position.x
                y: yaml.position.y
                onXChanged: yaml.position.x = x
                onYChanged: yaml.position.y = y

                function addPendingLink() {
                    var component = Qt.createComponent("PendingLink.qml");
                    component.createObject(ribbonRightRow, {
                                               text: edit.selectedText,
                                               linkUrl: Utils.rangedAnchorUrl(notepage.fileUrl,
                                                                              edit.selectedText),
                                               textEdit: edit
                                           })
                }

                function remove() {
                    folderModel.deleteFile(fileUrl)
                    peter.reload()
                }

                function save() {
                    yaml.saveToDocument()
                    edit.textDocument.save()
                }

                function setRandomPosition() {
                    console.log(notepage.fileName, notepage.x, notepage.y, "getting randomized")
                    notepage.x = Math.random() * (surfaceWindow.width - notepage.width) / 2 + (surface.width - surfaceWindow.width) / 2
                    notepage.y = Math.random() * (surfaceWindow.height - notepage.height) / 2 + (surface.height - surfaceWindow.height) / 2
                }

                property list<int> linkedIndices: folderModel.getLinkedIndices(index)

                Repeater {
                    model: notepage.linkedIndices

                    delegate: Shape {
                        required property int modelData     // index in folderModel
                        property Item otherPage: peter.itemAt(modelData)
                        property point otherPos: visible ? surface.mapToItem(notepage, otherPage.position) : startPoint
                        property bool otherBelow: otherPos.y > notepage.height / 2
                        property point startPoint: Qt.point(notepage.width / 2, notepage.height / 2)
                        property point endPoint: visible ? Qt.point(otherPos.x + otherPage.width / 2,
                                                                    otherPos.y + otherPage.height / 2) : startPoint
                        id: transpointer
                        opacity: 0.5
                        visible: otherPage
                        z: -1

                        ShapePath {
                            id: linkPath
                            strokeWidth: ribbon.height
                            strokeColor: "cyan"
                            fillColor: "transparent"
                            startX: transpointer.startPoint.x
                            startY: transpointer.startPoint.y

                            PathQuad {
                                x: endPoint.x
                                y: endPoint.y
                                relativeControlX: (x - linkPath.startX) / 2
                                relativeControlY: (y - linkPath.startY) / -2
                            }
                        }
                    }
                }

                DragHandler {
                    enabled: !flick.visible
                    onActiveChanged: if (active) notepage.z = ++surface.highestZ
                }

                Rectangle {
                    id: ribbon
                    x: 1; y: 1
                    width: parent.width - 2
                    height: title.implicitHeight + modTime.implicitHeight + 2
                    color: "papayawhip"
                    z: 1

                    DragHandler {
                        target: notepage
                        onActiveChanged: if (active) notepage.z = ++surface.highestZ
                    }

                    TextInput {
                        id: title
                        text: notepage.title
                        font.pointSize: 10 / Math.min(1, surface.scale * 1.5)
                        x: 3
                        anchors.bottom: parent.bottom
                        font.bold: true
                        onAccepted: folderModel.rename(fileUrl, title.text)
                    }

                    Row {
                        id: ribbonRightRow
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        anchors.margins: 1
                    }

                    Text {
                        id: modTime
                        text: Qt.formatDateTime(notepage.fileModified, Locale.LongFormat)
                        font.pointSize: 7
                        anchors.top: parent.top
                        anchors.right: parent.right
                        anchors.margins: 3
                        visible: surface.scale > 0.7
                    }

                    Text {
                        id: birthTime
                        text: Qt.formatDateTime(yaml.birth, Locale.LongFormat)
                        font.pointSize: 7
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.margins: 3
                        visible: surface.scale > 0.7
                    }

                    Text {
                        anchors.bottom: parent.bottom
                        anchors.right: parent.right
                        font.pointSize: 7
                        textFormat: Text.MarkdownText
                        text: "**" + index + "**: links to " + notepage.linkedIndices
                    }
                }

                Rectangle {
                    id: focusRect
                    z: 2
                    anchors.fill: parent
                    anchors.margins: 1
                    color: "transparent"
                    border.color: "saddlebrown"
                    border.width: 2
                    visible: edit.activeFocus
                }

                Flickable {
                    id: flick
                    anchors.fill: parent
                    anchors.margins: 5
                    anchors.topMargin: ribbon.height
                    contentWidth: edit.contentWidth
                    contentHeight: edit.contentHeight
                    clip: true
                    visible: surface.scale > 0.25

                    function ensureVisible(r) {
                        if (contentX >= r.x)
                            contentX = r.x;
                        else if (contentX+width <= r.x+r.width)
                            contentX = r.x+r.width-width;
                        if (contentY >= r.y)
                            contentY = r.y;
                        else if (contentY+height <= r.y+r.height)
                            contentY = r.y+r.height-height;
                    }

                    TextEdit {
                        id: edit
                        width: flick.width
                        textDocument.source: notepage.fileUrl
                        textDocument.onError: (message) => toastMessage.text += message + "\n"
                        // textFormat: TextEdit.MarkdownText
                        wrapMode: TextEdit.WordWrap
                        onLinkActivated: (link) => Qt.openUrlExternally(link) // TODO navigate internal links
                        onActiveFocusChanged:
                            if (!activeFocus && textDocument.modified)
                                notepage.save()
                    }
                }

                YamlDocument {
                    id: yaml
                    document: edit.textDocument
                    source: notepage.fileUrl
                    onParsed: {
                        if (!positionSet)
                            notepage.setRandomPosition()
                    }
                }

                Rectangle {
                    id: brTip
                    color: "#88FFFFFF"
                    border.color: "#55220000"
                    anchors.bottom: parent.bottom
                    anchors.right: parent.right
                    width: brText.implicitWidth + 4
                    height: brText.implicitHeight + 4
                    visible: brText.text.length > 0

                    Text {
                        id: brText
                        y: 2
                        anchors.right: parent.right
                        anchors.margins: 2
                        text: edit.hoveredLink
                    }
                }
            }
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
