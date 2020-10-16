# Generated from ndefeditor.pro.

cmake_minimum_required(VERSION 3.14)
project(ndefeditor LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/nfc/ndefeditor")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Nfc)
find_package(Qt6 COMPONENTS Widgets)

qt_add_executable(ndefeditor
    main.cpp
    mainwindow.cpp mainwindow.h mainwindow.ui
    mimeimagerecordeditor.cpp mimeimagerecordeditor.h mimeimagerecordeditor.ui
    textrecordeditor.cpp textrecordeditor.h textrecordeditor.ui
    urirecordeditor.cpp urirecordeditor.h urirecordeditor.ui
)
target_link_libraries(ndefeditor PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Nfc
    Qt::Widgets
)

install(TARGETS ndefeditor
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
