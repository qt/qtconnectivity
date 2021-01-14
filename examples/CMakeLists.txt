qt_examples_build_begin()

if(TARGET Qt::Bluetooth)
    add_subdirectory(bluetooth)
endif()
if(TARGET Qt::Nfc)
    add_subdirectory(nfc)
endif()

qt_examples_build_end()
