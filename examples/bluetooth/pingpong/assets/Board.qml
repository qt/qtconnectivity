// Copyright (C) 2014 BlackBerry Limited. All rights reserved.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick 2.0
import QtQuick.Window 2.1

Rectangle {
    id: fullWindow
    anchors.fill: parent
    color: "black"

    property double scaleFactor: Math.min(width, height)

    Rectangle {
        id: board
        width: scaleFactor
        height: scaleFactor
        anchors.centerIn: parent

        // Left pedal - server role
        Rectangle {
            id: leftblock
            y: (parent.height/2)
            width: (parent.width/27)
            height: (parent.height/5)
            anchors.left: parent.left
            color: "#363636"
            radius: width/2

            MouseArea {
                id: leftMouse
                width: (board.width/2)
                height: parent.height
                anchors.horizontalCenter: parent.horizontalCenter
                acceptedButtons: Qt.LeftButton
                drag.target: leftblock
                drag.axis: Drag.YAxis
                drag.minimumY: 0
                drag.maximumY: (board.height - leftblock.height)
            }
        }

        // Right pedal - client role
        Rectangle {
            id: rightblock
            y: (parent.height/2)
            width: (parent.width/27)
            height: (parent.height/5)
            anchors.right: parent.right
            color: "#363636"
            radius: width/2

            MouseArea {
                id: rightMouse
                width: (board.width/2)
                height: parent.height
                anchors.horizontalCenter: parent.horizontalCenter
                acceptedButtons: Qt.LeftButton
                drag.target: rightblock
                drag.axis: Drag.YAxis
                drag.minimumY: 0
                drag.maximumY: (board.height - rightblock.height)
            }
        }

        Rectangle {
            id: splitter
            color: "#363636"
            anchors.horizontalCenter: parent.horizontalCenter
            height: parent.height
            width: parent.width/100
        }

        Text {
            id: leftResult
            text: pingPong.leftResult
            font.bold: true
            font.pixelSize: 30
            anchors.right: splitter.left
            anchors.top: parent.top
            anchors.margins: 15
        }

        Text {
            id: rightResult
            text: pingPong.rightResult
            font.bold: true
            font.pixelSize: 30
            anchors.left: splitter.right
            anchors.top: parent.top
            anchors.margins: 15
        }

        Rectangle {
            id: ball
            width: leftblock.width
            height: leftblock.width
            radius: width/2
            color: "#363636"
            x: pingPong.ballX * scaleFactor
            y: pingPong.ballY * scaleFactor
        }
    }

    // 1 - server role; left pedal
    // 2 - client role; right pedal
    property int roleSide: pingPong.role
    onRoleSideChanged: {
        if (pingPong.role == 1) {
            rightMouse.opacity = 0.7
            rightMouse.enabled = false
        }
        else if (pingPong.role == 2) {
            leftMouse.opacity = 0.7
            leftMouse.enabled = false
        }
    }

    property bool deviceMessage: pingPong.showDialog
    onDeviceMessageChanged: {
        if (pingPong.showDialog) {
            info.visible = true;
            board.opacity = 0.5;
        } else {
            info.visible = false;
            board.opacity = 1;
        }
    }

    property double leftBlockY: leftblock.y
    onLeftBlockYChanged: pingPong.updateLeftBlock(leftblock.y / scaleFactor)

    property double leftBlockUpdate: pingPong.leftBlockY
    onLeftBlockUpdateChanged: leftblock.y = pingPong.leftBlockY * scaleFactor

    property double rightBlockY: rightblock.y
    onRightBlockYChanged: pingPong.updateRightBlock(rightblock.y / scaleFactor)

    property double rightBlockUpdate: pingPong.rightBlockY
    onRightBlockUpdateChanged: rightblock.y = pingPong.rightBlockY * scaleFactor


    Component.onCompleted: {
        pingPong.updateLeftBlock(leftblock.y / scaleFactor)
        pingPong.updateRightBlock(rightblock.y / scaleFactor)
    }
}
