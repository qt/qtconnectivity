if(NOT TARGET Qt::Test)
    cmake_minimum_required(VERSION 3.15.0)
    project(QtConnectivityTests VERSION 6.0.0 LANGUAGES C CXX)
    find_package(Qt6 REQUIRED COMPONENTS BuildInternals Core Bluetooth Nfc Test
                                                        Network)
    find_package(Qt6 OPTIONAL_COMPONENTS Gui Widgets)
    qt_set_up_standalone_tests_build()
endif()

if(TARGET Qt::Bluetooth)
    if(TARGET Qt::Quick)
        add_subdirectory(bttestui)
    endif()
endif()

qt_build_tests()
