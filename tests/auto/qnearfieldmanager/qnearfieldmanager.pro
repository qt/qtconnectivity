requires(contains(QT_CONFIG, private_tests))

SOURCES += tst_qnearfieldmanager.cpp
TARGET = tst_qnearfieldmanager
CONFIG += testcase

QT = core nfc-private testlib

INCLUDEPATH += ../../../src/nfc
VPATH += ../../../src/nfc

HEADERS += \
    qnearfieldmanager_emulator_p.h \
    qnearfieldtarget_emulator_p.h \
    targetemulator_p.h

SOURCES += \
    qnearfieldmanager_emulator.cpp \
    qnearfieldtarget_emulator.cpp \
    targetemulator.cpp

DEFINES += SRCDIR=\\\"$$PWD/../nfcdata\\\"

TESTDATA += "$$PWD/../nfcdata/Qt Labs Website Tag Type1.nfc" \
            "$$PWD/../nfcdata/Qt Website Tag Type1.nfc"

builtin_testdata: DEFINES += BUILTIN_TESTDATA
