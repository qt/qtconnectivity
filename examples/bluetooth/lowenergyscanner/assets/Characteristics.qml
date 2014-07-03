/***************************************************************************
**
** Copyright (C) 2013 BlackBerry Limited. All rights reserved.
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
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
        visible: false
    }

    ListView {
        id: characteristicview
        width: parent.width

        anchors.top: header.bottom
        anchors.bottom: menu.top
        model: device.characteristicList

        delegate: Rectangle {
            id: characteristicbox
            height:350
            width: parent.width

            Component.onCompleted: menu.menuText = "Back"

            Label {
                id: characteristicName1
                textContent: modelData.characteristicName
                anchors.top: parent.top
                anchors.topMargin: 5
            }

            Label {
                id: characteristicUuid1
                textContent: modelData.characteristicUuid
                anchors.verticalCenter: parent.verticalCenter
            }

            Label {
                id: characteristicValue
                textContent: ("Value: " + modelData.characteristicValue)
                anchors.bottom: characteristicHandle.top
                horizontalAlignment: Text.AlignHCenter
                anchors.topMargin: 5
            }

            Label {
                id: characteristicHandle
                textContent: ("Handlers: " + modelData.characteristicHandle)
                anchors.bottom: characteristicPermission.top
                anchors.topMargin: 5
            }

            Label {
                id: characteristicPermission
                textContent: modelData.characteristicPermission
                anchors.bottom: parent.bottom
                anchors.topMargin: 10
            }

            Rectangle {
                id: bottomarea
                anchors.bottom: characteristicbox.bottom
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
        menuText: device.update
        menuHeight: (parent.height/6)
        onButtonClick: {
            pageLoader.source = "Services.qml"
        }
    }
}
