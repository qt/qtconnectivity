// Copyright (C) 2014 BlackBerry Limited. All rights reserved.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick 2.0

Rectangle {
    anchors.fill: parent


    Rectangle {
        width: parent.width
        height: headerText.implicitHeight *1.2
        border.width: 1
        border.color: "#363636"
        radius: 5

        Text {
            id: headerText
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            anchors.fill: parent
            text: "Welcome to PingPong Game \n Please select an option"
            font.pointSize: 20
            elide: Text.ElideMiddle
            color: "#363636"
        }
    }

    Rectangle {
        id: startServer
        anchors.centerIn: parent
        width: parent.width/2
        height: startServerText.implicitHeight*5
        color: "#363636"

        Text {
            id: startServerText
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            anchors.fill: parent
            font.bold: true
            text: "Start PingPong server"
            color: "#E3E3E3"
            wrapMode: Text.WordWrap
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                pageLoader.source = "Board.qml";
                pingPong.startServer();
            }
        }
    }

    Rectangle {
        id: startClient
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: startServer.bottom
        anchors.topMargin: 10
        width: parent.width/2
        height: startClientText.implicitHeight*5
        color: "#363636"

        Text {
            id: startClientText
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            anchors.fill: parent
            font.bold: true
            text: "Start PingPong client"
            color: "#E3E3E3"
            wrapMode: Text.WordWrap
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                pageLoader.source = "Board.qml";
                pingPong.startClient()
            }
        }
    }
}
