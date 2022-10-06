# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_get_android_sdk_jar_for_api("android-18" android_sdk)

set(java_sources
    src/org/qtproject/qt/android/nfc/QtNfc.java
    src/org/qtproject/qt/android/nfc/QtNfcBroadcastReceiver.java
)

qt_internal_add_jar(Qt${QtConnectivity_VERSION_MAJOR}AndroidNfc
    INCLUDE_JARS ${android_sdk}
    SOURCES ${java_sources}
    OUTPUT_DIR "${QT_BUILD_DIR}/jar"
)

qt_path_join(destination ${INSTALL_DATADIR} "jar")

install_jar(Qt${QtConnectivity_VERSION_MAJOR}AndroidNfc
    DESTINATION ${destination}
    COMPONENT Devel
)

add_dependencies(Nfc Qt${QtConnectivity_VERSION_MAJOR}AndroidNfc)
