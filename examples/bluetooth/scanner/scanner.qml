/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the QtBluetooth module.
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
import QtBluetooth 5.0

Item {
    id: top

    property BluetoothService currentService

    BluetoothDiscoveryModel {
        id: btModel
        minimalDiscovery: true
        onDiscoveryChanged: console.log("Discovery mode: " + discovery)
        onNewServiceDiscovered: console.log("Found new service " + service.deviceAddress + " " + service.deviceName + " " + service.serviceName);
   }

    Rectangle {
        id: busy

        width: top.width * 0.8;
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: top.top;
        height: 20;
        radius: 5
        color: "#1c56f3"
        visible: btModel.discovery

        Text {
            id: text
            text: "Scanning"
            font.bold: true
            anchors.centerIn: parent
        }

        SequentialAnimation on color {
            id: busyThrobber
            ColorAnimation { easing.type: Easing.InOutSine; from: "#1c56f3"; to: "white"; duration: 1000; }
            ColorAnimation { easing.type: Easing.InOutSine; to: "#1c56f3"; from: "white"; duration: 1000 }
            loops: Animation.Infinite
        }
    }

    ListView {
        id: mainList
        width: top.width
        anchors.top: busy.bottom
        anchors.bottom: fullDiscoveryButton.top

        model: btModel
        delegate: Rectangle {
            id: btDelegate
            width: parent.width
            height: column.height + 10

            property bool expended: false;
            clip: true
            Image {
                id: bticon
                source: "qrc:/default.png";
                width: bttext.height;
                height: bttext.height;
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.margins: 5
            }

            Column {
                id: column
                anchors.left: bticon.right
                anchors.leftMargin: 5
                Text {
                    id: bttext
                    text: name;
                    font.family: "FreeSerif"
                    font.pointSize: 12
                }

                Text {
                    id: details
                    function get_details(s) {
                        var str = "Address: " + s.deviceAddress;
                        if (s.serviceName) { str += "<br>Service: " + s.serviceName; }
                        if (s.serviceDescription) { str += "<br>Description: " + s.serviceDescription; }
                        if (s.serviceProtocol) { str += "<br>Protocol: " + s.serviceProtocol; }
                        if (s.servicePort) { str += "<br>Port: " + s.servicePort; }
                        return str;
                    }
                    visible: opacity !== 0
                    opacity: btDelegate.expended ? 1 : 0.0
                    text: get_details(service)
                    font: bttext.font
                    Behavior on opacity {
                        NumberAnimation { duration: 200}
                    }
                }
            }
            Behavior on height { NumberAnimation { duration: 200} }

            MouseArea {
                anchors.fill: parent
                onClicked: btDelegate.expended = !btDelegate.expended
            }
        }
        focus: true
    }

    Rectangle {
        id: fullDiscoveryButton

        property bool fullDiscovery: false

        onFullDiscoveryChanged: {
            btModel.minimalDiscovery = !fullDiscovery;
            //reset discovery since we changed the discovery mode
            btModel.discovery = false;
            btModel.discovery = true;
        }

        anchors.bottom:  top.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 3

        height: 20

        color: fullDiscovery ? "#1c56f3" : "white"

        radius: 5
        border.width: 1

        Text {
            id: label
            text: "Full Discovery"
            anchors.centerIn: parent
        }

        MouseArea {
            anchors.fill: parent
            onClicked: parent.fullDiscovery = !parent.fullDiscovery
        }

        Behavior on color { ColorAnimation { duration: 200 } }
    }
}
