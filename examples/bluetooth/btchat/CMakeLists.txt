cmake_minimum_required(VERSION 3.16)
project(btchat LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/bluetooth/btchat")

find_package(Qt6 REQUIRED COMPONENTS Bluetooth Core Widgets)

qt_add_executable(btchat
    chat.cpp chat.h chat.ui
    chatclient.cpp chatclient.h
    chatserver.cpp chatserver.h
    main.cpp
    remoteselector.cpp remoteselector.h remoteselector.ui
)

set_target_properties(btchat PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(btchat PUBLIC
    Qt::Bluetooth
    Qt::Core
    Qt::Widgets
)

install(TARGETS btchat
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
