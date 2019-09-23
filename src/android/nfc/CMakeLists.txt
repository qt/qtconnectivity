# special case begin
# This is a manually edited file

set(path_prefix ${CMAKE_CURRENT_SOURCE_DIR}/src/org/qtproject/qt5/android/nfc)

set(java_sources
    ${path_prefix}/src/org/qtproject/qt5/android/nfc/QtNfc.java
    ${path_prefix}/src/org/qtproject/qt5/android/nfc/QtNfcBroadcastReceiver.java
)

add_jar(QtAndroidNfc
    INCLUDE_JARS ${QT_ANDROID_JAR}
    SOURCES ${java_sources}
    )

install_jar(QtAndroidNfc
    DESTINATION jar
    COMPONENT Devel)

# special case end
