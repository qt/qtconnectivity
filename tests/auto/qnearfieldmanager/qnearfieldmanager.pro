SOURCES += tst_qnearfieldmanager.cpp
TARGET = tst_qnearfieldmanager
CONFIG += testcase

QT = core nfc-private testlib

INCLUDEPATH += ../../../src/nfc

# This test includes source code from QtNfc library. Define the namespace which moc generated code
# should be in.
DEFINES += QT_BEGIN_MOC_NAMESPACE=\""namespace QtNfc {"\"
DEFINES += QT_END_MOC_NAMESPACE=\""}"\"

HEADERS += \
    qnearfieldmanagervirtualbase_p.h \
    qnearfieldmanager_emulator_p.h \
    qnearfieldtarget_emulator_p.h \
    targetemulator_p.h

SOURCES += \
    qnearfieldmanagervirtualbase.cpp \
    qnearfieldmanager_emulator.cpp \
    qnearfieldtarget_emulator.cpp \
    targetemulator.cpp

DEFINES += SRCDIR=\\\"$$PWD/../nfcdata\\\"
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0

