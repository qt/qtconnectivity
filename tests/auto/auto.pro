TEMPLATE = subdirs

SUBDIRS += \
    cmake

!isEmpty(QT.bluetooth.name) {
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

!isEmpty(QT.nfc.name) {
    SUBDIRS += \
        qndefmessage \
        qndefrecord \
        qnearfieldmanager \
        qnearfieldtagtype1 \
        qnearfieldtagtype2 \
}

qbluetoothservicediscoveryagent.CONFIG += no_check_target # QTBUG-22017
