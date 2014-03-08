/***************************************************************************
**
** Copyright (C) 2013 BlackBerry Limited. All rights reserved.
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.0

Rectangle {
    id: back
    width: 300
    height: 600
    property bool deviceState: device.state
    onDeviceStateChanged: {
        if (!device.state)
            info.visible = false;
    }

    Header {
        id: header
        anchors.top: parent.top
        headerText: "Start a Device Discovery"
    }

    Dialog {
        id: info
        anchors.centerIn: parent
        visible: false
    }

    ListView {
        id: theListView
        width: parent.width

        anchors.top: header.bottom
        anchors.bottom: menu.top
        model: device.devicesList

        delegate: Rectangle {
            id: box
            height:140
            width: parent.width
            Component.onCompleted: { info.visible = false;
                header.headerText = "Select a device";
            }

            MouseArea {
                anchors.fill: parent
                onPressed: { box.height= 135; downpart.height = 7}
                onReleased: { box.height= 140; downpart.height = 2}
                onClicked: {device.scanServices(modelData.deviceAddress); pageLoader.source = "Services.qml"}

            }

            Label {
                id: deviceName1
                textContent: modelData.deviceName
                anchors.top: parent.top
                anchors.topMargin: 5
            }

            Label {
                id: deviceAddress1
                textContent: modelData.deviceAddress
                anchors.bottom: downpart.top
                anchors.bottomMargin: 5
            }

            Rectangle {
                id: downpart
                anchors.bottom: box.bottom
                width: parent.width
                height: 2
                color: "#363636"
            }

        }
    }

    Menu {
        id: menu
        anchors.bottom: parent.bottom
        menuWidth: parent.width
        menuHeight: (parent.height/6)
        menuText: device.update
        onButtonClick: { device.startDeviceDiscovery(); info.dialogText = "Searching..."; info.visible = true;}
    }

    Loader {
        id: pageLoader
        anchors.fill: parent

    }
}
