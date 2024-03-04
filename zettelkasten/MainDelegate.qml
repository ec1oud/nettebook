// Copyright (C) 2024 Shawn Rutledge <s@ecloud.org>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Shapes
import Qt.labs.folderlistmodel
import org.nettebook.zettelkasten

Flipable {
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
        if (flipped) {
            sourceEdit.textDocument.save()
        } else {
            yaml.saveToDocument()
            edit.textDocument.save()
        }
    }

    function setRandomPosition() {
        console.log(notepage.fileName, notepage.x, notepage.y, "getting randomized")
        notepage.x = Math.random() * (surfaceWindow.width - notepage.width) / 2 + (surface.width - surfaceWindow.width) / 2
        notepage.y = Math.random() * (surfaceWindow.height - notepage.height) / 2 + (surface.height - surfaceWindow.height) / 2
    }

    front: Rectangle {
        id: yellowFront
        anchors.fill: parent
        color: "lightyellow"

        Repeater {
            id: linkRepeater

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
                preferredRendererType: Shape.CurveRenderer

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
                height: Math.max(flick.height, implicitHeight)
                // textDocument.source: notepage.fileUrl // racy
                textDocument.onModifiedChanged:
                    if (!textDocument.modified) { // loading is done
                        linkRepeater.model = folderModel.getLinkedIndices(notepage.index)
                    }
                textDocument.onErrorStringChanged: (str) => { if (str !== "") toastMessage.text += message + "\n" }
                textFormat: TextEdit.MarkdownText
                wrapMode: TextEdit.WordWrap
                onCursorRectangleChanged: flick.ensureVisible(cursorRectangle)
                onLinkActivated: (link) => Qt.openUrlExternally(link) // TODO navigate internal links
                onActiveFocusChanged:
                    if (!activeFocus && textDocument.modified)
                        notepage.save()
                Component.onCompleted: {
                    linkRepeater.model = null
                    edit.textDocument.source = notepage.fileUrl // workaround for QTBUG-120772
                }
            }
        }

        YamlDocument {
            id: yaml
            document: edit.textDocument
            onNeedsPosition: notepage.setRandomPosition()
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

        Text {
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.rightMargin: 4
            text: "⚙"
            font.pointSize: 18
            TapHandler {
                onTapped: notepage.flipped = true
            }
        }
    } // front

    back: Rectangle {
        anchors.fill: parent
        color: "#222"
        border.color: "lightgreen"

        Flickable {
            id: sourceFlick
            anchors.fill: parent
            anchors.margins: 5
            contentWidth: sourceEdit.contentWidth
            contentHeight: sourceEdit.contentHeight
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
                id: sourceEdit
                width: Math.max(flick.width, implicitWidth)
                height: Math.max(flick.height, implicitHeight)
                // textDocument.source: notepage.fileUrl // racy
                textFormat: TextEdit.PlainText
                font.family: "monospace"
                color: "white"
                onCursorRectangleChanged: flick.ensureVisible(cursorRectangle)
                onActiveFocusChanged:
                    if (!activeFocus && textDocument.modified)
                        notepage.save()
                Component.onCompleted: sourceEdit.textDocument.source = notepage.fileUrl // workaround for QTBUG-120772
            }

        }
        Rectangle {
            color: "#333"
            border.color: "black"
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: 1
            width: flipBackIcon.implicitHeight + 4
            height: width
            topLeftRadius: 8
            Text {
                id: flipBackIcon
                anchors.centerIn: parent
                text: "✎"
                font.pointSize: 18
                color: "lightgreen"
                TapHandler {
                    onTapped: notepage.flipped = false
                }
            }
        }
    } // back

    property bool flipped: false

    transform: Rotation {
        id: rotation
        origin.x: notepage.width / 2
        origin.y: notepage.height / 2
        axis.x: 0; axis.y: 1; axis.z: 0     // set axis.y to 1 to rotate around y-axis
        angle: 0    // the default angle
    }

    states: State {
        name: "back"
        PropertyChanges { target: rotation; angle: 180 }
        when: notepage.flipped
    }

    transitions: Transition {
        NumberAnimation { target: rotation; property: "angle"; duration: 500 }
    }

}
