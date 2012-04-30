TEMPLATE = subdirs

SUBDIRS += \
    qbluetoothaddress \
    qbluetoothdevicediscoveryagent \
    qbluetoothdeviceinfo \
    qbluetoothlocaldevice \
    qbluetoothservicediscoveryagent \
    qbluetoothserviceinfo \
    qbluetoothsocket \
    qbluetoothtransfermanager \
    qbluetoothtransferrequest \
    qbluetoothuuid \
    ql2capserver \
#    qndefmessage \
#    qndefrecord \
#    qnearfieldmanager \
#    qnearfieldtagtype1 \
#    qnearfieldtagtype2 \
    qrfcommserver \

!contains(QT_CONFIG, bluetooth): SUBDIRS -= \
    qbluetoothaddress \
    qbluetoothdevicediscoveryagent \
    qbluetoothdeviceinfo \
    qbluetoothlocaldevice \
    qbluetoothservicediscoveryagent \
    qbluetoothserviceinfo \
    qbluetoothsocket \
    qbluetoothtransfermanager \
    qbluetoothtransferrequest \
    qbluetoothuuid \
    ql2capserver \

#!contains(QT_CONFIG, nfc): SUBDIRS -= \
#    qndefmessage \
#    qndefrecord \
#    qnearfieldmanager \
#    qnearfieldtagtype1 \
#    qnearfieldtagtype2 \
#
## QTBUG-22015 these do not compile.
#SUBDIRS -= \
#    qnearfieldmanager \
#    qnearfieldtagtype1 \
#    qnearfieldtagtype2 \

qbluetoothservicediscoveryagent.CONFIG += no_check_target # QTBUG-22017
