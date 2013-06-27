TEMPLATE = subdirs

SUBDIRS += \
    cmake

qtHaveModule(bluetooth) {
    SUBDIRS += \
        qbluetoothaddress \
        qbluetoothdevicediscoveryagent \
        qbluetoothdeviceinfo \
        qbluetoothlocaldevice \
        qbluetoothhostinfo \
        qbluetoothservicediscoveryagent \
        qbluetoothserviceinfo \
        qbluetoothsocket \
        qbluetoothtransfermanager \
        qbluetoothtransferrequest \
        qbluetoothuuid \
        ql2capserver \
        qrfcommserver
}

qtHaveModule(nfc) {
    SUBDIRS += \
        qndefmessage \
        qndefrecord \
        qnearfieldmanager \
        qnearfieldtagtype1 \
        qnearfieldtagtype2 \
        qndefnfcsmartposterrecord
}
