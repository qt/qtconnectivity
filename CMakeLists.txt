cmake_minimum_required(VERSION 3.15.0)

include(.cmake.conf)
project(QtConnectivity
    VERSION 6.0.0
    DESCRIPTION "Qt Connectivity Libraries"
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)

set(QT_REPO_NOT_WARNINGS_CLEAN TRUE)
find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals Core Network
                                                               Concurrent)
find_package(Qt6 ${PROJECT_VERSION} CONFIG OPTIONAL_COMPONENTS DBus Gui Widgets Quick)
if(ANDROID)
find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS AndroidExtras)
endif()

if(NOT TARGET Qt::Network)
    message(NOTICE "Skipping the build as the condition \"TARGET Qt::Network\" is not met.")
    return()
endif()
if(ANDROID AND NOT TARGET Qt::AndroidExtras)
    message(NOTICE "Skipping the build as the condition \"TARGET Qt::AndroidExtras OR NOT ANDROID\" is not met.")
    return()
endif()
qt_build_repo()
