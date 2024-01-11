// Copyright (C) 2024 Shawn Rutledge <s@ecloud.org>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
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

        Repeater {
            model: FolderListModel {
                id: folderModel
                folder: "file:example"
                showDirs: false
            }

            delegate: Rectangle {
                required property date fileModified
                required property string fileName
                required property url fileUrl
                property alias edit: edit
                id: notepage
                objectName: "frame: " + fileName
                color: "lightyellow"
                width: Math.max(320, title.implicitWidth); height: 240

                function addPendingLink() {
                    var component = Qt.createComponent("PendingLink.qml");
                    component.createObject(ribbonRightRow, {
                                               text: edit.selectedText,
                                               linkUrl: Utils.rangedAnchorUrl(notepage.fileUrl,
                                                                              edit.selectedText),
                                               textEdit: edit
                                           })
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

                    Text {
                        id: title
                        text: notepage.fileName
                        font.pointSize: 10 / Math.min(1, surface.scale * 1.5)
                        style: Text.Outline
                        styleColor: ribbon.color
                        x: 3; width: parent.width - 5
                        anchors.bottom: parent.bottom
                        font.bold: true
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
                        visible: surface.scale > 0.25
                        width: flick.width
                        textDocument.source: notepage.fileUrl
                        // textFormat: TextEdit.MarkdownText
                        wrapMode: TextEdit.WordWrap
                        onLinkActivated: (link) => Qt.openUrlExternally(link) // TODO navigate internal links
                        onActiveFocusChanged:
                            if (!activeFocus && textDocument.modified) {
                                yaml.saveToDocument()
                                textDocument.save()
                            }
                    }
                }

                YamlDocument {
                    id: yaml
                    document: edit.textDocument
                    source: notepage.fileUrl
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

                Component.onCompleted: {
                    x = Math.random() * (surfaceWindow.width - width) / 2 + (surface.width - surfaceWindow.width) / 2
                    y = Math.random() * (surfaceWindow.height - height) / 2 + (surface.height - surfaceWindow.height) / 2
                }
            }
        } // Repeater

        WheelHandler {
            property: "scale"
        }
    }


    QuadPieMenu {
        id: pieMenu
        labels: [ "", "🔗", "", "" ]
        onTriggered:
            (text) => {
                const p = surface.mapFromItem(pieMenu.parent, pieMenu.point.position)
                const ctxItem = surface.childAt(p.x, p.y)
                var ctxEdit = ctxItem?.edit
                // surfaceWindow.contentItem.dumpItemTree()
                console.log(text, "context menu on", ctxItem, ctxEdit)
                if (text === "🔗" && ctxEdit !== undefined)
                    ctxItem.addPendingLink()
            }
    }

    Shortcut { sequence: StandardKey.Quit; onActivated: Qt.quit() }
}
