# Generated from qtconnectivity.pro.

cmake_minimum_required(VERSION 3.15.0)

include(.cmake.conf)
project(QtConnectivity # special case
    VERSION 6.0.0
    DESCRIPTION "Qt Connectivity Libraries" # special case
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)

# special case begin
set(QT_REPO_NOT_WARNINGS_CLEAN TRUE)
find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals Core Gui Network
                                                               Concurrent Widgets)
find_package(Qt6 ${PROJECT_VERSION} CONFIG OPTIONAL_COMPONENTS DBus)
if(ANDROID)
find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS AndroidExtras)
endif()
# special case end

if(NOT TARGET Qt::Network)
    message(NOTICE "Skipping the build as the condition \"TARGET Qt::Network\" is not met.")
    return()
endif()
if(ANDROID AND NOT TARGET Qt::AndroidExtras)
    message(NOTICE "Skipping the build as the condition \"TARGET Qt::AndroidExtras OR NOT ANDROID\" is not met.")
    return()
endif()
qt_build_repo()
