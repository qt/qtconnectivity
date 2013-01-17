SOURCES += tst_qnearfieldtagtype2.cpp
TARGET = tst_qnearfieldtagtype2
CONFIG += testcase

QT = core nfc-private testlib

INCLUDEPATH += ../../../src/nfc
VPATH += ../../../src/nfc

# This test includes source code from QtNfc library. Define the namespace which moc generated code
# should be in.
DEFINES += QT_BEGIN_MOC_NAMESPACE=\""namespace QtNfc {"\"
DEFINES += QT_END_MOC_NAMESPACE=\""}"\"

HEADERS += \
    qnearfieldmanagervirtualbase_p.h \
    qnearfieldtarget_emulator_p.h \
    qnearfieldmanager_emulator_p.h \
    targetemulator_p.h

SOURCES += \
    qnearfieldmanagervirtualbase.cpp \
    qnearfieldtarget_emulator.cpp \
    qnearfieldmanager_emulator.cpp \
    targetemulator.cpp

DEFINES += SRCDIR=\\\"$$PWD\\\"
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0

# This test crashes
CONFIG += insignificant_test
