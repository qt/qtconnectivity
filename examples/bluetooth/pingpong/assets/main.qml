// Copyright (C) 2014 BlackBerry Limited. All rights reserved.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick 2.1
import QtQuick.Window 2.1

Window {
    id: menulist
    width: 600
    height: 600
    visibility: (Qt.platform.os === "android" || Qt.platform.os === "ios") ? Window.FullScreen : Window.Windowed
    visible: true

    Dialog {
        id: info
        anchors.centerIn: parent
        visible: false
    }

    Component.onCompleted: pageLoader.source = "Menu.qml"

    Loader {
        id: pageLoader
        anchors.fill: parent
    }
}
