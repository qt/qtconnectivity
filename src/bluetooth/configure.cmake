

#### Inputs



#### Libraries

qt_find_package(BlueZ PROVIDED_TARGETS PkgConfig::BlueZ)


#### Tests


if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../config.tests/bluez_le/CMakeLists.txt")
    qt_config_compile_test("bluez_le"
                           LABEL "BlueZ Low Energy"
                           PROJECT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../config.tests/bluez_le")
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../config.tests/linux_crypto_api/CMakeLists.txt")
    qt_config_compile_test("linux_crypto_api"
                           LABEL "Linux Crypto API"
                           PROJECT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../config.tests/linux_crypto_api")
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../config.tests/winrt_bt/CMakeLists.txt")
    qt_config_compile_test("winrt_bt"
                           LABEL "WinRT Bluetooth API"
                           PROJECT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../config.tests/winrt_bt")
endif()


#### Features

qt_feature("bluez" PUBLIC
    LABEL "BlueZ"
    CONDITION BLUEZ_FOUND AND QT_FEATURE_dbus # special case
)
qt_feature("bluez_le" PRIVATE
    LABEL "BlueZ Low Energy"
    CONDITION QT_FEATURE_bluez AND TEST_bluez_le
)
qt_feature("linux_crypto_api" PRIVATE
    LABEL "Linux Crypto API"
    CONDITION QT_FEATURE_bluez_le AND TEST_linux_crypto_api
)
qt_feature("winrt_bt" PRIVATE
    LABEL "WinRT Bluetooth API (desktop & UWP)"
    CONDITION WIN32 AND TEST_winrt_bt
)
