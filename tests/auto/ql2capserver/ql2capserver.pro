SOURCES += tst_ql2capserver.cpp
TARGET = tst_ql2capserver

# Note, not really an autotest.  Requires manually setting up and running btclient first.
#CONFIG += testcase

QT = core concurrent bluetooth testlib

symbian: TARGET.CAPABILITY =  LocalServices NetworkControl
