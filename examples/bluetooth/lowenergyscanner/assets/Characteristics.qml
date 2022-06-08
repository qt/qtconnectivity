// Copyright (C) 2013 BlackBerry Limited. All rights reserved.
// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick 2.0

Rectangle {
    width: 300
    height: 600

    Header {
        id: header
        anchors.top: parent.top
        headerText: "Characteristics list"
    }

    Dialog {
        id: info
        anchors.centerIn: parent
        visible: true
        dialogText: "Scanning for characteristics...";
    }

    Connections {
        target: device
        function onCharacteristicsUpdated() {
            menu.menuText = "Back"
            if (characteristicview.count === 0) {
                info.dialogText = "No characteristic found"
                info.busyImage = false
            } else {
                info.visible = false
                info.busyImage = true
            }
        }

        function onDisconnected() {
            pageLoader.source = "main.qml"
        }
    }

    ListView {
        id: characteristicview
        width: parent.width
        clip: true

        anchors.top: header.bottom
        anchors.bottom: menu.top
        model: device.characteristicList

        delegate: Rectangle {
            id: characteristicbox
            height:300
            width: characteristicview.width
            color: "lightsteelblue"
            border.width: 2
            border.color: "black"
            radius: 5

            Label {
                id: characteristicName
                textContent: modelData.characteristicName
                anchors.top: parent.top
                anchors.topMargin: 5
            }

            Label {
                id: characteristicUuid
                font.pointSize: characteristicName.font.pointSize*0.7
                textContent: modelData.characteristicUuid
                anchors.top: characteristicName.bottom
                anchors.topMargin: 5
            }

            Label {
                id: characteristicValue
                font.pointSize: characteristicName.font.pointSize*0.7
                textContent: ("Value: " + modelData.characteristicValue)
                anchors.bottom: characteristicHandle.top
                horizontalAlignment: Text.AlignHCenter
                anchors.topMargin: 5
            }

            Label {
                id: characteristicHandle
                font.pointSize: characteristicName.font.pointSize*0.7
                textContent: ("Handlers: " + modelData.characteristicHandle)
                anchors.bottom: characteristicPermission.top
                anchors.topMargin: 5
            }

            Label {
                id: characteristicPermission
                font.pointSize: characteristicName.font.pointSize*0.7
                textContent: modelData.characteristicPermission
                anchors.bottom: parent.bottom
                anchors.topMargin: 5
                anchors.bottomMargin: 5
            }
        }
    }

    Menu {
        id: menu
        anchors.bottom: parent.bottom
        menuWidth: parent.width
        menuText: device.update
        menuHeight: (parent.height/6)
        onButtonClick: {
            pageLoader.source = "Services.qml"
            device.update = "Back"
        }
    }
}
