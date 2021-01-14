#####################################################################
## tst_qlowenergyservice Test:
#####################################################################

qt_internal_add_test(tst_qlowenergyservice
    SOURCES
        tst_qlowenergyservice.cpp
    PUBLIC_LIBRARIES
        Qt::Bluetooth
)
