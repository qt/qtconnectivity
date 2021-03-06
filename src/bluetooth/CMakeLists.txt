#####################################################################
## Bluetooth Module:
#####################################################################

qt_internal_add_module(Bluetooth
    SOURCES
        lecmaccalculator_p.h
        qbluetooth.cpp qbluetooth.h
        qbluetoothaddress.cpp qbluetoothaddress.h
        qbluetoothdevicediscoveryagent.cpp qbluetoothdevicediscoveryagent.h qbluetoothdevicediscoveryagent_p.h
        qbluetoothdeviceinfo.cpp qbluetoothdeviceinfo.h qbluetoothdeviceinfo_p.h
        qbluetoothhostinfo.cpp qbluetoothhostinfo.h qbluetoothhostinfo_p.h
        qbluetoothlocaldevice.cpp qbluetoothlocaldevice.h qbluetoothlocaldevice_p.h
        qbluetoothserver.cpp qbluetoothserver.h qbluetoothserver_p.h
        qbluetoothservicediscoveryagent.cpp qbluetoothservicediscoveryagent.h qbluetoothservicediscoveryagent_p.h
        qbluetoothserviceinfo.cpp qbluetoothserviceinfo.h qbluetoothserviceinfo_p.h
        qbluetoothsocket.cpp qbluetoothsocket.h
        qbluetoothsocketbase.cpp qbluetoothsocketbase_p.h
        qbluetoothuuid.cpp qbluetoothuuid.h
        qleadvertiser_p.h
        qlowenergyadvertisingdata.cpp qlowenergyadvertisingdata.h
        qlowenergyadvertisingparameters.cpp qlowenergyadvertisingparameters.h
        qlowenergycharacteristic.cpp qlowenergycharacteristic.h
        qlowenergycharacteristicdata.cpp qlowenergycharacteristicdata.h
        qlowenergyconnectionparameters.cpp qlowenergyconnectionparameters.h
        qlowenergycontroller.cpp qlowenergycontroller.h
        qlowenergycontrollerbase.cpp qlowenergycontrollerbase_p.h
        qlowenergydescriptor.cpp qlowenergydescriptor.h
        qlowenergydescriptordata.cpp qlowenergydescriptordata.h
        qlowenergyservice.cpp qlowenergyservice.h
        qlowenergyservicedata.cpp qlowenergyservicedata.h
        qlowenergyserviceprivate.cpp qlowenergyserviceprivate_p.h
        qprivatelinearbuffer_p.h
        qtbluetoothglobal.h qtbluetoothglobal_p.h
    DEFINES
        QT_NO_FOREACH
    LIBRARIES
        Qt::CorePrivate
        Qt::NetworkPrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Network
    PRIVATE_MODULE_INTERFACE
        Qt::CorePrivate
        Qt::NetworkPrivate
)

#### Keys ignored in scope 1:.:.:bluetooth.pro:<TRUE>:
# OTHER_FILES = "doc/src/*.qdoc"

## Scopes:
#####################################################################

#### Keys ignored in scope 2:.:.:bluetooth.pro:WIN32:
# WINDOWS_SDK_VERSION = "$$member($$list $$split WINDOWS_SDK_VERSION_STRING, . , 2)"
# WINDOWS_SDK_VERSION_STRING = "$$(WindowsSDKVersion)"

if (WIN32)
    if (DEFINED CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION)
        set (WINDOWS_SDK_VERSION ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION})
    elseif (DEFINED CMAKE_SYSTEM_VERSION)
        set (WINDOWS_SDK_VERSION ${CMAKE_SYSTEM_VERSION})
    endif ()
    if (DEFINED WINDOWS_SDK_VERSION)
        string(REGEX MATCHALL "[0-9]+\\.|[0-9]+$" VERSIONLIST ${WINDOWS_SDK_VERSION})
        list(GET VERSIONLIST 2 WINDOWS_SDK_VERSION_PATCH)
    endif ()
endif ()

qt_internal_extend_target(Bluetooth CONDITION WIN32 AND WINDOWS_SDK_VERSION_PATCH
    DEFINES
        QT_UCRTVERSION="${WINDOWS_SDK_VERSION_PATCH}"
)

qt_internal_extend_target(Bluetooth CONDITION WIN32 AND WINDOWS_SDK_VERSION_PATCH LESS 14393
    DEFINES
        QT_WINRT_LIMITED_SERVICEDISCOVERY
)

qt_internal_extend_target(Bluetooth CONDITION QT_FEATURE_bluez
    SOURCES
        bluez/adapter.cpp bluez/adapter_p.h
        bluez/adapter1_bluez5.cpp bluez/adapter1_bluez5_p.h
        bluez/battery1.cpp bluez/battery1_p.h
        bluez/bluetoothmanagement.cpp bluez/bluetoothmanagement_p.h
        bluez/bluez5_helper.cpp bluez/bluez5_helper_p.h
        bluez/bluez_data_p.h
        bluez/device.cpp bluez/device_p.h
        bluez/device1_bluez5.cpp bluez/device1_bluez5_p.h
        bluez/gattchar1.cpp bluez/gattchar1_p.h
        bluez/gattdesc1.cpp bluez/gattdesc1_p.h
        bluez/gattservice1.cpp bluez/gattservice1_p.h
        bluez/hcimanager.cpp bluez/hcimanager_p.h
        bluez/manager.cpp bluez/manager_p.h
        bluez/objectmanager.cpp bluez/objectmanager_p.h
        bluez/profile1.cpp bluez/profile1_p.h
        bluez/profile1context.cpp bluez/profile1context_p.h
        bluez/profilemanager1.cpp bluez/profilemanager1_p.h
        bluez/properties.cpp bluez/properties_p.h
        bluez/remotedevicemanager.cpp bluez/remotedevicemanager_p.h
        bluez/service.cpp bluez/service_p.h
        bluez/servicemap.cpp bluez/servicemap_p.h
        qbluetoothdevicediscoveryagent_bluez.cpp
        qbluetoothlocaldevice_bluez.cpp
        qbluetoothserver_bluez.cpp
        qbluetoothservicediscoveryagent_bluez.cpp
        qbluetoothserviceinfo_bluez.cpp
        qbluetoothsocket_bluez.cpp qbluetoothsocket_bluez_p.h
        qbluetoothsocket_bluezdbus.cpp qbluetoothsocket_bluezdbus_p.h
    PUBLIC_LIBRARIES # for Linux QEMU (gcc-armv7) on Linux Ubuntu_20_04 (gcc-x86_64)
        Qt::DBus
)

qt_internal_extend_target(Bluetooth CONDITION QT_FEATURE_bluez AND QT_FEATURE_bluez_le
    SOURCES
        lecmaccalculator.cpp
        qleadvertiser_bluez.cpp
        qlowenergycontroller_bluez.cpp qlowenergycontroller_bluez_p.h
        qlowenergycontroller_bluezdbus.cpp qlowenergycontroller_bluezdbus_p.h
)

qt_internal_extend_target(Bluetooth CONDITION QT_FEATURE_bluez AND QT_FEATURE_bluez_le AND QT_FEATURE_linux_crypto_api
    DEFINES
        CONFIG_LINUX_CRYPTO_API
)

qt_internal_extend_target(Bluetooth CONDITION QT_FEATURE_bluez AND NOT QT_FEATURE_bluez_le
    SOURCES
        dummy/dummy_helper.cpp dummy/dummy_helper_p.h
        qlowenergycontroller_dummy.cpp qlowenergycontroller_dummy_p.h
    DEFINES
        QT_BLUEZ_NO_BTLE
)

if(ANDROID AND (ANDROID AND NOT ANDROID_EMBEDDED))
    set_property(TARGET Bluetooth APPEND PROPERTY QT_ANDROID_BUNDLED_JAR_DEPENDENCIES
        jar/Qt${QtConnectivity_VERSION_MAJOR}AndroidBluetooth.jar:org.qtproject.qt.android.bluetooth.QtBluetoothBroadcastReceiver
    )
    set_property(TARGET Bluetooth APPEND PROPERTY QT_ANDROID_PERMISSIONS
        android.permission.ACCESS_COARSE_LOCATION android.permission.ACCESS_FINE_LOCATION android.permission.BLUETOOTH android.permission.BLUETOOTH_ADMIN
    )
endif()

qt_internal_extend_target(Bluetooth CONDITION ANDROID AND NOT ANDROID_EMBEDDED AND NOT QT_FEATURE_bluez
    SOURCES
        android/androidbroadcastreceiver.cpp android/androidbroadcastreceiver_p.h
        android/devicediscoverybroadcastreceiver.cpp android/devicediscoverybroadcastreceiver_p.h
        android/inputstreamthread.cpp android/inputstreamthread_p.h
        android/jni_android.cpp android/jni_android_p.h
        android/localdevicebroadcastreceiver.cpp android/localdevicebroadcastreceiver_p.h
        android/lowenergynotificationhub.cpp android/lowenergynotificationhub_p.h
        android/serveracceptancethread.cpp android/serveracceptancethread_p.h
        android/servicediscoverybroadcastreceiver.cpp android/servicediscoverybroadcastreceiver_p.h
        qbluetoothdevicediscoveryagent_android.cpp
        qbluetoothlocaldevice_android.cpp
        qbluetoothserver_android.cpp
        qbluetoothservicediscoveryagent_android.cpp
        qbluetoothserviceinfo_android.cpp
        qbluetoothsocket_android.cpp qbluetoothsocket_android_p.h
        qlowenergycontroller_android.cpp qlowenergycontroller_android_p.h
    DEFINES
        QT_ANDROID_BLUETOOTH
    LIBRARIES
        Qt::AndroidExtras
        Qt::CorePrivate
    PRIVATE_MODULE_INTERFACE
        Qt::AndroidExtras
        Qt::CorePrivate
)

qt_internal_extend_target(Bluetooth CONDITION MACOS AND NOT QT_FEATURE_bluez AND (ANDROID_EMBEDDED OR NOT ANDROID)
    SOURCES
        darwin/btcentralmanager.mm darwin/btcentralmanager_p.h
        darwin/btconnectionmonitor.mm darwin/btconnectionmonitor_p.h
        darwin/btdelegates.cpp darwin/btdelegates_p.h
        darwin/btdeviceinquiry.mm darwin/btdeviceinquiry_p.h
        darwin/btdevicepair.mm darwin/btdevicepair_p.h
        darwin/btgcdtimer.mm darwin/btgcdtimer_p.h
        darwin/btl2capchannel.mm darwin/btl2capchannel_p.h
        darwin/btledeviceinquiry.mm darwin/btledeviceinquiry_p.h
        darwin/btnotifier.cpp darwin/btnotifier_p.h
        darwin/btobexsession.mm darwin/btobexsession_p.h
        darwin/btperipheralmanager.mm darwin/btperipheralmanager_p.h
        darwin/btraii.mm darwin/btraii_p.h
        darwin/btrfcommchannel.mm darwin/btrfcommchannel_p.h
        darwin/btsdpinquiry.mm darwin/btsdpinquiry_p.h
        darwin/btservicerecord.mm darwin/btservicerecord_p.h
        darwin/btsocketlistener.mm darwin/btsocketlistener_p.h
        darwin/btutility.mm darwin/btutility_p.h
        darwin/uistrings.cpp darwin/uistrings_p.h
        qbluetoothdevicediscoveryagent_darwin.mm
        qbluetoothlocaldevice_macos.mm
        qbluetoothserver_macos.mm
        qbluetoothservicediscoveryagent_macos.mm
        qbluetoothserviceinfo_macos.mm
        qbluetoothsocket_macos.mm qbluetoothsocket_macos_p.h
        qlowenergycontroller_darwin.mm qlowenergycontroller_darwin_p.h
    DEFINES
        QT_OSX_BLUETOOTH
    LIBRARIES
        ${FWFoundation}
        ${FWIOBluetooth}
)

qt_internal_extend_target(Bluetooth CONDITION NOT MACOS AND NOT QT_FEATURE_bluez AND (IOS OR TVOS) AND (ANDROID_EMBEDDED OR NOT ANDROID)
    SOURCES
        darwin/btcentralmanager.mm darwin/btcentralmanager_p.h
        darwin/btdelegates.cpp darwin/btdelegates_p.h
        darwin/btgcdtimer.mm darwin/btgcdtimer_p.h
        darwin/btledeviceinquiry.mm darwin/btledeviceinquiry_p.h
        darwin/btnotifier.cpp darwin/btnotifier_p.h
        darwin/btraii.mm darwin/btraii_p.h
        darwin/btutility.mm darwin/btutility_p.h
        darwin/uistrings.cpp darwin/uistrings_p.h
        qbluetoothdevicediscoveryagent_darwin.mm
        qbluetoothlocaldevice_p.cpp
        qbluetoothserver_p.cpp
        qbluetoothservicediscoveryagent_p.cpp
        qbluetoothserviceinfo_p.cpp
        qbluetoothsocket_dummy.cpp qbluetoothsocket_dummy_p.h
        qlowenergycontroller_darwin.mm qlowenergycontroller_darwin_p.h
    DEFINES
        QT_IOS_BLUETOOTH
    LIBRARIES
        ${FWCoreBluetooth}
        ${FWFoundation}
)

qt_internal_extend_target(Bluetooth CONDITION IOS AND NOT MACOS AND NOT QT_FEATURE_bluez AND (ANDROID_EMBEDDED OR NOT ANDROID)
    SOURCES
        darwin/btperipheralmanager.mm darwin/btperipheralmanager_p.h
)

qt_internal_extend_target(Bluetooth CONDITION QT_FEATURE_winrt_bt AND NOT IOS AND NOT MACOS AND NOT QT_FEATURE_bluez AND NOT TVOS AND (ANDROID_EMBEDDED OR NOT ANDROID)
    SOURCES
        qbluetoothdevicediscoveryagent_winrt.cpp
        qbluetoothlocaldevice_winrt.cpp
        qbluetoothserver_winrt.cpp
        qbluetoothservicediscoveryagent_winrt.cpp
        qbluetoothserviceinfo_winrt.cpp
        qbluetoothsocket_winrt.cpp qbluetoothsocket_winrt_p.h
        qbluetoothutils_winrt.cpp qbluetoothutils_winrt_p.h
        qlowenergycontroller_winrt.cpp qlowenergycontroller_winrt_p.h
    DEFINES
        QT_WINRT_BLUETOOTH
    LIBRARIES
        Qt::CorePrivate
    PUBLIC_LIBRARIES
        Qt::Core
        runtimeobject.lib
        user32.lib
    PRIVATE_MODULE_INTERFACE
        Qt::CorePrivate
)

qt_internal_extend_target(Bluetooth CONDITION QT_FEATURE_winrt_bt AND QT_FEATURE_winrt_btle_no_pairing AND NOT IOS AND NOT MACOS AND NOT QT_FEATURE_bluez AND NOT TVOS AND (ANDROID_EMBEDDED OR NOT ANDROID)
    SOURCES
        qlowenergycontroller_winrt_new.cpp qlowenergycontroller_winrt_new_p.h
)

qt_internal_extend_target(Bluetooth CONDITION WIN32 AND NOT QT_FEATURE_bluez AND NOT QT_FEATURE_winrt_bt AND (ANDROID_EMBEDDED OR NOT ANDROID)
    SOURCES
        qbluetoothdevicediscoveryagent_win.cpp
        qbluetoothlocaldevice_win.cpp
        qbluetoothserver_win.cpp
        qbluetoothservicediscoveryagent_win.cpp
        qbluetoothserviceinfo_win.cpp
        qbluetoothsocket_win.cpp qbluetoothsocket_win_p.h
        qlowenergycontroller_win.cpp qlowenergycontroller_win_p.h
        windows/qwinlowenergybluetooth_p.h
    DEFINES
        QT_WIN_BLUETOOTH
    PUBLIC_LIBRARIES
        bthprops
        setupapi
        ws2_32
)

qt_internal_extend_target(Bluetooth CONDITION UNIX AND NOT IOS AND NOT MACOS AND NOT QT_FEATURE_bluez AND NOT QT_FEATURE_winrt_bt AND NOT TVOS AND (ANDROID_EMBEDDED OR NOT ANDROID)
    SOURCES
        dummy/dummy_helper.cpp dummy/dummy_helper_p.h
        qbluetoothdevicediscoveryagent_p.cpp
        qbluetoothlocaldevice_p.cpp
        qbluetoothserver_p.cpp
        qbluetoothservicediscoveryagent_p.cpp
        qbluetoothserviceinfo_p.cpp
        qbluetoothsocket_dummy.cpp qbluetoothsocket_dummy_p.h
        qlowenergycontroller_dummy.cpp qlowenergycontroller_dummy_p.h
)

qt_internal_add_docs(Bluetooth
    doc/qtbluetooth.qdocconf
)

