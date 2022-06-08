// Copyright (C) 2013 BlackBerry Limited. All rights reserved.
// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick 2.0

Rectangle {
    width: 300
    height: 600

    Component.onCompleted: {
        // Loading this page may take longer than QLEController
        // stopping with an error, go back and readjust this view
        // based on controller errors
        if (device.controllerError) {
            info.visible = false;
            menu.menuText = device.update
        }
    }

    Header {
        id: header
        anchors.top: parent.top
        headerText: "Services list"
    }

    Dialog {
        id: info
        anchors.centerIn: parent
        visible: true
        dialogText: "Scanning for services...";
    }

    Connections {
        target: device
        function onServicesUpdated() {
            if (servicesview.count === 0)
                info.dialogText = "No services found"
            else
                info.visible = false;
        }

        function onDisconnected() {
            pageLoader.source = "main.qml"
        }
    }

    ListView {
        id: servicesview
        width: parent.width
        anchors.top: header.bottom
        anchors.bottom: menu.top
        model: device.servicesList
        clip: true

        delegate: Rectangle {
            id: servicebox
            height:100
            color: "lightsteelblue"
            border.width: 2
            border.color: "black"
            radius: 5
            width: servicesview.width
            Component.onCompleted: {
                info.visible = false
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    pageLoader.source = "Characteristics.qml";
                    device.connectToService(modelData.serviceUuid);
                }
            }

            Label {
                id: serviceName
                textContent: modelData.serviceName
                anchors.top: parent.top
                anchors.topMargin: 5
            }

            Label {
                textContent: modelData.serviceType
                font.pointSize: serviceName.font.pointSize * 0.5
                anchors.top: serviceName.bottom
            }

            Label {
                id: serviceUuid
                font.pointSize: serviceName.font.pointSize * 0.5
                textContent: modelData.serviceUuid
                anchors.bottom: servicebox.bottom
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
            device.disconnectFromDevice()
            pageLoader.source = "main.qml"
            device.update = "Search"
        }
    }
}
