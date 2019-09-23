# Generated from qbluetoothtransfermanager.pro.

#####################################################################
## tst_qbluetoothtransfermanager Test:
#####################################################################

# Collect test data
file(GLOB_RECURSE test_data_glob
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    *.txt)
list(APPEND test_data ${test_data_glob})

add_qt_test(tst_qbluetoothtransfermanager
    SOURCES
        tst_qbluetoothtransfermanager.cpp
    PUBLIC_LIBRARIES
        Qt::Bluetooth
        Qt::Concurrent
    TESTDATA ${test_data}
)

#### Keys ignored in scope 1:.:.:qbluetoothtransfermanager.pro:<TRUE>:
# testcase.timeout = "250"
