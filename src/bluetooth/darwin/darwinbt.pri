SOURCES += darwin/uistrings.cpp \
           darwin/btnotifier.cpp \
           darwin/btdelegates.cpp \
           darwin/btledeviceinquiry.mm \
           darwin/btcentralmanager.mm

HEADERS += darwin/uistrings_p.h \
                   darwin/btgcdtimer_p.h \
                   darwin/btraii_p.h \
                   darwin/btdelegates_p.h \
                   darwin/btutility_p.h \
                   darwin/btledeviceinquiry_p.h \
                   darwin/btcentralmanager_p.h \
                   darwin/btnotifier_p.h

OBJECTIVE_SOURCES += darwin/btgcdtimer.mm \
                     darwin/btraii.mm \
                     darwin/btutility.mm

#QMAKE_CXXFLAGS_WARN_ON += -Wno-nullability-completeness

macos {
    HEADERS += darwin/btdevicepair_p.h \
                       darwin/btdeviceinquiry_p.h \
                       darwin/btconnectionmonitor_p.h \
                       darwin/btsdpinquiry_p.h \
                       darwin/btrfcommchannel_p.h \
                       darwin/btl2capchannel_p.h \
                       darwin/btservicerecord_p.h \
                       darwin/btsocketlistener_p.h \
                       darwin/btobexsession_p.h

    OBJECTIVE_SOURCES += darwin/btdevicepair.mm \
                         darwin/btdeviceinquiry.mm \
                         darwin/btconnectionmonitor.mm \
                         darwin/btsdpinquiry.mm \
                         darwin/btrfcommchannel.mm \
                         darwin/btl2capchannel.mm \
                         darwin/btservicerecord.mm \
                         darwin/btsocketlistener.mm \
                         darwin/btobexsession.mm
}

macos | ios {
    HEADERS += darwin/btperipheralmanager_p.h

    OBJECTIVE_SOURCES += darwin/btperipheralmanager.mm
}
