if(QT_BUILD_STANDALONE_TESTS)
    # Add qt_find_package calls for extra dependencies that need to be found when building
    # the standalone tests here.
endif()

if(TARGET Qt::Bluetooth)
    if(TARGET Qt::Quick)
        add_subdirectory(bttestui)
    endif()

    add_subdirectory(bluetoothtestdevice)
endif()

qt_build_tests()
