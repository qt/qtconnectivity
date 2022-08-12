cmake_minimum_required(VERSION 3.16)
project(heartrate-game LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/bluetooth/heartrate-game")

find_package(Qt6 REQUIRED COMPONENTS Bluetooth Core Gui Qml Quick)

qt_add_executable(heartrate-game
    bluetoothbaseclass.cpp bluetoothbaseclass.h
    connectionhandler.cpp connectionhandler.h
    devicefinder.cpp devicefinder.h
    devicehandler.cpp devicehandler.h
    deviceinfo.cpp deviceinfo.h
    heartrate-global.h
    main.cpp
)

set_target_properties(heartrate-game PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(heartrate-game PUBLIC
    Qt::Bluetooth
    Qt::Core
    Qt::Gui
    Qt::Qml
    Qt::Quick
)

qt_add_qml_module(heartrate-game
    URI Shared
    VERSION 1.0
    NO_RESOURCE_TARGET_PATH
)

if (APPLE)
    if (IOS)
        set_target_properties(heartrate-game PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist"
        )
    else()
        # Using absolute path for shared plist files is a Ninja bug workaround
        get_filename_component(SHARED_PLIST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../shared ABSOLUTE)
        set_target_properties(heartrate-game PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST "${SHARED_PLIST_DIR}/Info.cmake.macos.plist"
        )
    endif()
endif()

# Resources:
set(qml_resource_files
    "qml/App.qml"
    "qml/BluetoothAlarmDialog.qml"
    "qml/BottomLine.qml"
    "qml/Connect.qml"
    "qml/GameButton.qml"
    "qml/GamePage.qml"
    "qml/GameSettings.qml"
    "qml/Measure.qml"
    "qml/SplashScreen.qml"
    "qml/Stats.qml"
    "qml/StatsLabel.qml"
    "qml/TitleBar.qml"
    "qml/main.qml"
    "qml/qmldir"
)

qt6_add_resources(heartrate-game "qml"
    PREFIX
        "/"
    FILES
        ${qml_resource_files}
)

set(images_resource_files
    "qml/images/bt_off_to_on.png"
    "qml/images/heart.png"
    "qml/images/logo.png"
)

qt6_add_resources(heartrate-game "images"
    PREFIX
        "/"
    FILES
        ${images_resource_files}
)

install(TARGETS heartrate-game
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
