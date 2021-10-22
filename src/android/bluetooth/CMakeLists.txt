qt_get_android_sdk_jar_for_api("android-21" android_sdk)

set(java_sources
    src/org/qtproject/qt/android/bluetooth/QtBluetoothBroadcastReceiver.java
    src/org/qtproject/qt/android/bluetooth/QtBluetoothInputStreamThread.java
    src/org/qtproject/qt/android/bluetooth/QtBluetoothLE.java
    src/org/qtproject/qt/android/bluetooth/QtBluetoothLEServer.java
    src/org/qtproject/qt/android/bluetooth/QtBluetoothSocketServer.java
    src/org/qtproject/qt/android/bluetooth/QtBluetoothGattCharacteristic.java
)

qt_internal_add_jar(Qt${QtConnectivity_VERSION_MAJOR}AndroidBluetooth
    INCLUDE_JARS ${android_sdk}
    SOURCES ${java_sources}
    OUTPUT_DIR "${QT_BUILD_DIR}/jar"
)

install_jar(Qt${QtConnectivity_VERSION_MAJOR}AndroidBluetooth
    DESTINATION jar
    COMPONENT Devel
)

add_dependencies(Bluetooth Qt${QtConnectivity_VERSION_MAJOR}AndroidBluetooth)
