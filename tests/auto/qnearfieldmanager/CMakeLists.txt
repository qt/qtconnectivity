if (NOT QT_FEATURE_private_tests)
    return()
endif()

#####################################################################
## tst_qnearfieldmanager Test:
#####################################################################

# Collect test data
list(APPEND test_data "../nfcdata/Qt Labs Website Tag Type1.nfc")
list(APPEND test_data "../nfcdata/Qt Website Tag Type1.nfc")

qt_internal_add_test(tst_qnearfieldmanager
    SOURCES
        ../../../src/nfc/qnearfieldmanager_emulator.cpp ../../../src/nfc/qnearfieldmanager_emulator_p.h
        ../../../src/nfc/qnearfieldtarget_emulator.cpp ../../../src/nfc/qnearfieldtarget_emulator_p.h
        ../../../src/nfc/targetemulator.cpp ../../../src/nfc/targetemulator_p.h
        tst_qnearfieldmanager.cpp
    DEFINES
        SRCDIR=\\\"${CMAKE_CURRENT_SOURCE_DIR}/../nfcdata\\\"
    INCLUDE_DIRECTORIES
        ../../../src/nfc
    PUBLIC_LIBRARIES
        Qt::NfcPrivate
    TESTDATA ${test_data}
)

#### Keys ignored in scope 1:.:.:qnearfieldmanager.pro:<TRUE>:
# _REQUIREMENTS = "contains(QT_CONFIG, private_tests)"

## Scopes:
#####################################################################

qt_internal_extend_target(tst_qnearfieldmanager CONDITION builtin_testdata
    DEFINES
        BUILTIN_TESTDATA
)
