// Copyright (C) 2024 Shawn Rutledge <s@ecloud.org>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import Qt.labs.folderlistmodel

Window {
    id: surface
    width: 3840; height: 2160
    visibility: Window.Maximized
    flags: Qt.CustomizeWindowHint | Qt.FramelessWindowHint
    color: "#00000000"
    title: qsTr("Nettebook Zettelkasten")
    property int highestZ: 0

    Image {
        anchors.fill: parent
        source: "resources/gridsquare.png"
        fillMode: Image.Tile
    }

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
            id: notepage
            objectName: "frame-" + fileName
            color: "lightyellow"
            width: 320; height: 240
            clip: true

            Rectangle {
                id: ribbon
                width: parent.width
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
                    x: 2; width: parent.width - 4
                    anchors.bottom: parent.bottom
                    font.bold: true
                }

                Text {
                    id: modTime
                    text: Qt.formatDateTime(notepage.fileModified, Locale.LongFormat)
                    font.pointSize: 7
                    anchors.right: parent.right
                    anchors.margins: 2
                }
            }

            Flickable {
                id: flick
                anchors.fill: parent
                anchors.margins: 2
                anchors.topMargin: ribbon.height + 4
                contentWidth: edit.contentWidth
                contentHeight: edit.contentHeight

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
                    wrapMode: TextEdit.WordWrap
                    onLinkActivated: (link) => Qt.openUrlExternally(link) // TODO navigate internal links
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

            Component.onCompleted: {
                x = Math.random() * (surface.width - width) / 2
                y = Math.random() * (surface.height - height) / 2
            }
        }
    }

    Shortcut { sequence: StandardKey.Quit; onActivated: Qt.quit() }
}
