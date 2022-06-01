cmake_minimum_required(VERSION 3.16)
project(ndefeditor LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/nfc/ndefeditor")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Nfc Widgets)

qt_add_executable(ndefeditor
    MANUAL_FINALIZATION
    main.cpp
    mainwindow.cpp mainwindow.h mainwindow.ui
    mimeimagerecordeditor.cpp mimeimagerecordeditor.h mimeimagerecordeditor.ui
    textrecordeditor.cpp textrecordeditor.h textrecordeditor.ui
    urirecordeditor.cpp urirecordeditor.h urirecordeditor.ui
)

set_target_properties(ndefeditor PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

if(IOS)
    set_target_properties(ndefeditor PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist"
    )
endif()

target_link_libraries(ndefeditor PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Nfc
    Qt::Widgets
)

if(ANDROID)
    set_property(TARGET ndefeditor
        APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/android
    )
endif()

qt_finalize_target(ndefeditor)

install(TARGETS ndefeditor
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
