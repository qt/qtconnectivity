# Generated from nfc.pro.

qt_get_android_sdk_jar_for_api("android-18" android_sdk)

set(java_sources
    src/org/qtproject/qt5/android/nfc/QtNfc.java
    src/org/qtproject/qt5/android/nfc/QtNfcBroadcastReceiver.java
)

add_jar(QtAndroidNfc # special case
    INCLUDE_JARS ${android_sdk}
    SOURCES ${java_sources}
)

install_jar(QtAndroidNfc # special case
    DESTINATION jar
    COMPONENT Devel
)
