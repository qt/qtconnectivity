cmake_minimum_required(VERSION 3.16)

include(.cmake.conf)
project(QtConnectivity
    VERSION "${QT_REPO_MODULE_VERSION}"
    DESCRIPTION "Qt Connectivity Libraries"
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)

# Make sure we only use latest private CMake API, aka no compatibility wrappers.
set(QT_NO_INTERNAL_COMPATIBILITY_FUNCTIONS TRUE)

# Make sure we use the fixed BASE argument of qt_add_resource.
set(QT_USE_FIXED_QT_ADD_RESOURCE_BASE TRUE)

set(QT_REPO_NOT_WARNINGS_CLEAN TRUE)
find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals Core Network)
find_package(Qt6 ${PROJECT_VERSION} CONFIG OPTIONAL_COMPONENTS DBus Gui Widgets Quick)

if(NOT TARGET Qt::Network)
    message(NOTICE "Skipping the build as the condition \"TARGET Qt::Network\" is not met.")
    return()
endif()

qt_build_repo()
