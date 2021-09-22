cmake_minimum_required(VERSION 3.16)

include(.cmake.conf)
project(QtConnectivity
    VERSION "${QT_REPO_MODULE_VERSION}"
    DESCRIPTION "Qt Connectivity Libraries"
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)

set(QT_REPO_NOT_WARNINGS_CLEAN TRUE)
find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals Core Network)
find_package(Qt6 ${PROJECT_VERSION} CONFIG OPTIONAL_COMPONENTS DBus Gui Widgets Quick)

if(NOT TARGET Qt::Network)
    message(NOTICE "Skipping the build as the condition \"TARGET Qt::Network\" is not met.")
    return()
endif()

qt_build_repo()
