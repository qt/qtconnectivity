# Generated from qtconnectivity.pro.

cmake_minimum_required(VERSION 3.15.0)

project(QtConnectivity # special case
    VERSION 6.0.0
    DESCRIPTION "Qt Connectivity Libraries" # special case
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)

# special case begin
find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals Core Gui Network
                                                               Concurrent Widgets)
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
