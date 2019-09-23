# special case begin
if(NOT TARGET Qt::Test)
    cmake_minimum_required(VERSION 3.15.0)
    project(QtConnectivityTests VERSION 6.0.0 LANGUAGES C CXX)
    find_package(Qt6 REQUIRED COMPONENTS BuildInternals Core Gui Concurrent Bluetooth Nfc Test
                                                        Network Widgets)
    qt_set_up_standalone_tests_build()
endif()
# special case end
qt_build_tests()
