SOURCES += tst_ql2capserver.cpp
TARGET = tst_ql2capserver

# Note, not really an autotest.  Requires manually setting up and running btclient first.
#CONFIG += testcase

QT = core bluetooth testlib

symbian: TARGET.CAPABILITY =  LocalServices NetworkControl

!include(../auto.pri):error(missing auto.pri)
