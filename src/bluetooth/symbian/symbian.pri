
contains(SYMBIAN_VERSION, Symbian3) {
	message("Extended Inquiry Request supported")
	DEFINES += EIR_SUPPORTED
}

PRIVATE_HEADERS += \
    bluetooth/symbian/bluetoothlinkmanagerdevicediscoverer.h \
    bluetooth/symbian/utils_symbian_p.h

SOURCES += \
    bluetooth/symbian/bluetoothlinkmanagerdevicediscoverer.cpp

contains(btengconnman_symbian_enabled, yes) {
    DEFINES += USING_BTENGCONNMAN
    LIBS *=-lbtengconnman
    PRIVATE_HEADERS += \
        bluetooth/symbian/bluetoothsymbianpairingadapter.h
    SOURCES += \
        bluetooth/symbian/bluetoothsymbianpairingadapter.cpp
}

contains(btengdevman_symbian_enabled, yes) {
    DEFINES += USING_BTENGDEVMAN
    LIBS *=-lbtengdevman
    PRIVATE_HEADERS += \
        bluetooth/symbian/bluetoothsymbianregistryadapter.h
    SOURCES += \
        bluetooth/symbian/bluetoothsymbianregistryadapter.cpp
}
