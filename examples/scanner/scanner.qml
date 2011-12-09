/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Mobility Components.
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
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
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
** $QT_END_LICENSE$
**
****************************************************************************/

import Qt 4.7
import QtMobility.connectivity 1.2

Rectangle {
    id: top

    property BluetoothService currentService
    property alias minimalDiscovery: myModel.minimalDiscovery
    property alias uuidFilder: myModel.uuidFilter

    anchors.fill: parent

    BluetoothDiscoveryModel {
        id: myModel
        minimalDiscovery: true
        onDiscoveryChanged: busy.running = discovery;
//        onNewServiceDiscovered: console.log("Found new service " + service.deviceAddress + " " + service.deviceName + " " + service.serviceName);
//        uuidFilter: "e8e10f95-1a70-4b27-9ccf-02010264e9c9"
   }

    Rectangle {
        id: busy
        property bool running: true

        width: top.width/1.2;
        x: top.width/2-width/2
        anchors.top: top.top;
        height: 20;
        radius: 5
        color: "#1c56f3"

        Text {
            id: text
            text: "<b>Scanning</b>"
            x: busy.width/2-paintedWidth/2
            y: busy.height/2-paintedHeight/2
        }

        SequentialAnimation on color {
            id: busyThrobber
            ColorAnimation { easing.type: Easing.InOutSine; from: "#1c56f3"; to: "white"; duration: 1000; }
            ColorAnimation { easing.type: Easing.InOutSine; to: "#1c56f3"; from: "white"; duration: 1000 }
            loops: Animation.Infinite
        }
        states: [
            State {
                name: "stopped"
                when: !busy.running
                PropertyChanges { target: busy; height: 0; }
                PropertyChanges { target: busyThrobber; running: false }
                PropertyChanges { target: text; visible: false }
            }
        ]
        transitions: [
            Transition {
                from: "*"
                to: "stopped"
                reversible: true
                NumberAnimation { target: busy; property: "height"; to: 0; duration: 200 }
            }
        ]
    }

    Component {
        id: del;

        Item {
            id: item

            function item_clicked(service) {
                console.log("Clicked " + service.deviceName + service.deviceAddress);
                top.currentService = service;
            }

            property int text_height: 5+(bticon.height > bttext.height ? bticon.height : bttext.height)

            height: text_height
            width: parent.width

            Column {
                anchors.fill: item
                Row {
                    width: parent.width
                    Image {
                        id: bticon
                        source: icon;
                        width: del.height;
                    }
                    Text {
                        id: bttext
                        text: name;
                        font.family: "FreeSerif"
                        font.pointSize: 12
                    }
                }
                Text {
                    function get_details(s) {
                        var str = "Address: " + s.deviceAddress;
                        if (s.serviceName) { str += "<br>Service: " + s.serviceName; }
                        if (s.serviceDescription) { str += "<br>Description: " + s.serviceDescription; }
                        if (s.serviceProtocol) { str += "<br>Port: " + s.serviceProtocol; }
                        if (s.servicePort) { str += "<br>Port: " + s.servicePort; }
                        return str;
                    }

                    id: details
                    opacity: 0.0
                    text: get_details(service)
                    font: bttext.font
                    x: bticon.width
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: item.state = ((item.state == "details") ? "" : "details")
            }

            states: [
                State {
                    name: "details"
                    PropertyChanges { target: item; height: item.text_height+details.height; }
                    PropertyChanges { target: details; opacity: 1.0 }
                }
            ]
            transitions: [
                Transition {
                    from: "*"
                    to: "details"
                    reversible: true
                    NumberAnimation { target: item; property: "height"; duration: 200 }
                    NumberAnimation { target: details; property: "lopacity"; duration: 200 }
                }
            ]
        }
    }

    Component {
        id: highlightBox

        Rectangle {
            id: background
            anchors.fill: del
            border.color: "#34ca57"
            radius: 5
            border.width: 2
        }

    }

    ListView {
        id: mainList
        width: top.width
        anchors.top: busy.bottom
//        anchors.bottom: top.bottom
        anchors.bottom: fullbutton.top

        model: myModel
        highlight: highlightBox
        delegate: del
        focus: true
    }

    Rectangle {
        id: fullbutton

        function button_clicked() {
            myModel.minimalDiscovery = !myModel.minimalDiscovery;
            fullbutton.state = fullbutton.state == "clicked" ? "" : "clicked";
            myModel.setDiscovery(true);
        }

        anchors.bottom:  top.bottom
        anchors.margins: 3

        width: top.width-6
        x: 3
        height: 20

        radius: 5
        border.width: 1
        color: "white"

        Text {
            id: label
            text: "Full Discovery"
            x: parent.width/2-paintedWidth/2;
            y: parent.height/2-paintedHeight/2;
        }

        MouseArea {
            anchors.fill: parent
            onClicked: fullbutton.button_clicked();
        }

        states: [
            State {
                name: "clicked"
                PropertyChanges { target: fullbutton; color: "#1c56f3" }
            }
        ]
        transitions: [
            Transition {
                from: "*"
                to: "details"
                reversible: true
                ColorAnimation { duration: 200 }
            }
        ]
    }
}
