TEMPLATE = subdirs

# Required by tst_maketestselftest::tests_pro_files
# Mark the following non unit test directories ok
# nfcdata

SUBDIRS += \
        qndefrecord \
        qndefmessage \
        qnearfieldtagtype1 \
        qnearfieldtagtype2 \
        qnearfieldmanager

#!win32:SUBDIRS += \
#        qbluetoothaddress\
#        qbluetoothuuid\
#        qbluetoothdeviceinfo\
#        qbluetoothdevicediscoveryagent\
#        qbluetoothserviceinfo\
#        qbluetoothservicediscoveryagent\
#        qbluetoothsocket\
#        qrfcommserver \
#        qbluetoothtransferrequest

#enable autotests on symbian
symbian:SUBDIRS += \
        qbluetoothaddress\
        qbluetoothuuid\
        qbluetoothdeviceinfo\
        qbluetoothdevicediscoveryagent\
        qbluetoothserviceinfo\
        qbluetoothservicediscoveryagent\
        qbluetoothsocket\
        qrfcommserver \
        qbluetoothtransferrequest \
        qbluetoothlocaldevice \
        qbluetoothtransfermanager \
        ql2capserver
