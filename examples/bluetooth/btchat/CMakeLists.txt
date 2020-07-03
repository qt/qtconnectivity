# Generated from btchat.pro.

cmake_minimum_required(VERSION 3.14)
project(btchat LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(INSTALL_EXAMPLEDIR "examples/bluetooth/btchat")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Bluetooth)
find_package(Qt6 COMPONENTS Widgets)

add_qt_gui_executable(btchat
    chat.cpp chat.h chat.ui
    chatclient.cpp chatclient.h
    chatserver.cpp chatserver.h
    main.cpp
    remoteselector.cpp remoteselector.h remoteselector.ui
)
target_link_libraries(btchat PUBLIC
    Qt::Bluetooth
    Qt::Core
    Qt::Widgets
)

if(ANDROID)
    target_link_libraries(btchat PUBLIC
        Qt::AndroidExtras
    )
endif()

install(TARGETS btchat
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
