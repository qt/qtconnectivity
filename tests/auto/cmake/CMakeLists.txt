
cmake_minimum_required(VERSION 2.8)

project(qmake_cmake_files)

enable_testing()

find_package(Qt5Core REQUIRED)

include("${_Qt5CTestMacros}")

set(qt_module_includes
    Bluetooth QBluetoothAddress
    Nfc QNdefRecord
)
set(QBluetoothAddress_NAMESPACE QtBluetooth)
set(QNdefRecord_NAMESPACE QtNfc)

test_module_includes(
    ${qt_module_includes}
)
