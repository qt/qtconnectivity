# Generated from qnearfieldtagtype2.pro.

# special case begin
if (NOT QT_FEATURE_private_tests)
    return()
endif()
# special case end

#####################################################################
## tst_qnearfieldtagtype2 Test:
#####################################################################

# Collect test data
list(APPEND test_data "nfcdata/Dynamic Empty Tag.nfc") # special case
list(APPEND test_data "nfcdata/Empty Tag.nfc") # special case

qt_internal_add_test(tst_qnearfieldtagtype2
    SOURCES
        ../../../src/nfc/qnearfieldmanager_emulator.cpp ../../../src/nfc/qnearfieldmanager_emulator_p.h
        ../../../src/nfc/qnearfieldtarget_emulator.cpp ../../../src/nfc/qnearfieldtarget_emulator_p.h
        ../../../src/nfc/targetemulator.cpp ../../../src/nfc/targetemulator_p.h
        tst_qnearfieldtagtype2.cpp
    DEFINES
        SRCDIR=\\\"${CMAKE_CURRENT_SOURCE_DIR}/nfcdata\\\"
    INCLUDE_DIRECTORIES
        ../../../src/nfc
    PUBLIC_LIBRARIES
        Qt::NfcPrivate
    TESTDATA ${test_data}
)

#### Keys ignored in scope 1:.:.:qnearfieldtagtype2.pro:<TRUE>:
# _REQUIREMENTS = "contains(QT_CONFIG, private_tests)"

## Scopes:
#####################################################################

qt_internal_extend_target(tst_qnearfieldtagtype2 CONDITION builtin_testdata
    DEFINES
        BUILTIN_TESTDATA
)
