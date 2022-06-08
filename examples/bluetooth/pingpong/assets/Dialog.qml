// Copyright (C) 2014 BlackBerry Limited. All rights reserved.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick 2.0

Rectangle {
    width: parent.width/2
    height: message.implicitHeight*2
    z: 50
    border.width: 2
    border.color: "#363636"
    radius: 10

    Text {
        id: message
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        anchors.fill: parent
        wrapMode: Text.WordWrap
        elide: Text.ElideMiddle
        text: pingPong.message
        color: "#363636"
    }
}
