/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Mobility Components.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtTest/QtTest>

#include <QDebug>
#include <QVariant>

#include <qbluetoothaddress.h>
#include <qbluetoothlocaldevice.h>

QTCONNECTIVITY_USE_NAMESPACE

#define WAIT_FOR_CONDITION(a,e)            \
    for (int _i = 0; _i < 5000; _i += 1) {  \
        if ((a) == (e)) break;             \
        QTest::qWait(100);}

class tst_QBluetoothLocalDevice : public QObject
{
    Q_OBJECT

public:
    tst_QBluetoothLocalDevice();
    ~tst_QBluetoothLocalDevice();

private slots:
    void tst_powerOn();
    void tst_powerOff();
    void tst_hostModes();
    void tst_hostModes_data();
    void tst_address();
    void tst_name();
    void tst_isValid();
    void tst_allDevices();
    void tst_construction();
    void tst_pairingStatus_data();
    void tst_pairingStatus();
    void tst_pairDevice_data();
    void tst_pairDevice();
};

tst_QBluetoothLocalDevice::tst_QBluetoothLocalDevice()
{
    // start with host powered off
    QBluetoothLocalDevice *device = new QBluetoothLocalDevice();
    device->setHostMode(QBluetoothLocalDevice::HostPoweredOff);
    delete device;
    // wait for the device to switch bluetooth mode.
    QTest::qWait(1000);

}

tst_QBluetoothLocalDevice::~tst_QBluetoothLocalDevice()
{
}

void tst_QBluetoothLocalDevice::tst_hostModes_data()
{
    QTest::addColumn<QBluetoothLocalDevice::HostMode>("hostModeExpected");

    QTest::newRow("HostDiscoverable") << QBluetoothLocalDevice::HostDiscoverable;
    QTest::newRow("HostPoweredOff") << QBluetoothLocalDevice::HostPoweredOff;
    QTest::newRow("HostConnectable") << QBluetoothLocalDevice::HostConnectable;
    QTest::newRow("HostPoweredOff") << QBluetoothLocalDevice::HostPoweredOff;
    QTest::newRow("HostDiscoverable") << QBluetoothLocalDevice::HostDiscoverable;

}
void tst_QBluetoothLocalDevice::tst_pairDevice_data()
{
    QTest::addColumn<QBluetoothAddress>("deviceAddress");
    QTest::addColumn<QBluetoothLocalDevice::Pairing>("pairingExpected");

    QTest::newRow("UnPaired Device: DUMMY") << QBluetoothAddress("11:00:00:00:00:00")
            << QBluetoothLocalDevice::Unpaired;
#ifdef Q_OS_SYMBIAN

    QTest::newRow("unPAIRED Device: J X6") << QBluetoothAddress("d8:75:33:6a:82:85")
            << QBluetoothLocalDevice::Unpaired;
    QTest::newRow("AuthPAIRED Device: J X6") << QBluetoothAddress("d8:75:33:6a:82:85")
            << QBluetoothLocalDevice::AuthorizedPaired;
    QTest::newRow("PAIRED Device: J C-7-1") << QBluetoothAddress("6c:9b:02:0c:91:ca")
            << QBluetoothLocalDevice::Paired;

#endif // Q_OS_SYMBIAN
}

void tst_QBluetoothLocalDevice::tst_pairingStatus_data()
{
    QTest::addColumn<QBluetoothAddress>("deviceAddress");
    QTest::addColumn<QBluetoothLocalDevice::Pairing>("pairingExpected");

    QTest::newRow("UnPaired Device: DUMMY") << QBluetoothAddress("11:00:00:00:00:00")
            << QBluetoothLocalDevice::Unpaired;
#ifdef Q_OS_SYMBIAN
    QTest::newRow("PAIRED Device: J X6") << QBluetoothAddress("d8:75:33:6a:82:85")
            << QBluetoothLocalDevice::Paired;
    QTest::newRow("AuthPAIRED Device: J C-7-1") << QBluetoothAddress("6c:9b:02:0c:91:ca")
            << QBluetoothLocalDevice::AuthorizedPaired;
#endif // Q_OS_SYMBIAN
}

void tst_QBluetoothLocalDevice::tst_powerOn()
{
    {
    QBluetoothLocalDevice localDevice;

    QSignalSpy hostModeSpy(&localDevice, SIGNAL(hostModeStateChanged(QBluetoothLocalDevice::HostMode)));
    // there should be no changes yet
    QVERIFY(hostModeSpy.isEmpty());

    localDevice.powerOn();
    // async, wait for it
    WAIT_FOR_CONDITION(hostModeSpy.count(),1);
    QVERIFY(hostModeSpy.count() > 0);
    QBluetoothLocalDevice::HostMode hostMode= localDevice.hostMode();
    // we should not be powered off
    QVERIFY(hostMode == QBluetoothLocalDevice::HostConnectable
         || hostMode == QBluetoothLocalDevice::HostDiscoverable);
    }

}
void tst_QBluetoothLocalDevice::tst_powerOff()
{
    {
        QBluetoothLocalDevice *device = new QBluetoothLocalDevice();
        device->powerOn();
        delete device;
        // wait for the device to switch bluetooth mode.
        QTest::qWait(1000);
    }
    QBluetoothLocalDevice localDevice;
    QSignalSpy hostModeSpy(&localDevice, SIGNAL(hostModeStateChanged(QBluetoothLocalDevice::HostMode)));
    // there should be no changes yet
    QVERIFY(hostModeSpy.isEmpty());

    localDevice.setHostMode(QBluetoothLocalDevice::HostPoweredOff);
    // async, wait for it
    WAIT_FOR_CONDITION(hostModeSpy.count(),1);
    QVERIFY(hostModeSpy.count() > 0);
    // we should not be powered off
    QVERIFY(localDevice.hostMode() == QBluetoothLocalDevice::HostPoweredOff);

}

void tst_QBluetoothLocalDevice::tst_hostModes()
{
    QFETCH(QBluetoothLocalDevice::HostMode, hostModeExpected);

    QBluetoothLocalDevice localDevice;
    QSignalSpy hostModeSpy(&localDevice, SIGNAL(hostModeStateChanged(QBluetoothLocalDevice::HostMode)));
    // there should be no changes yet
    QVERIFY(hostModeSpy.isEmpty());

    QTest::qWait(1000);

    localDevice.setHostMode(hostModeExpected);
    // wait for the device to switch bluetooth mode.
    QTest::qWait(1000);
    if (hostModeExpected != localDevice.hostMode()) {
        WAIT_FOR_CONDITION(hostModeSpy.count(),1);
        QVERIFY(hostModeSpy.count() > 0);
    }
    // test the actual signal values.
    QList<QVariant> arguments = hostModeSpy.takeFirst();
    QBluetoothLocalDevice::HostMode hostMode = qvariant_cast<QBluetoothLocalDevice::HostMode>(arguments.at(0));
    QCOMPARE(hostModeExpected, hostMode);
    // test actual
    QCOMPARE(hostModeExpected, localDevice.hostMode());
}

void tst_QBluetoothLocalDevice::tst_address()
{
    QBluetoothLocalDevice localDevice;
    QVERIFY(!localDevice.address().toString().isEmpty());
}
void tst_QBluetoothLocalDevice::tst_name()
{
    QBluetoothLocalDevice localDevice;
    QVERIFY(!localDevice.name().isEmpty());
}
void tst_QBluetoothLocalDevice::tst_isValid()
{
    QBluetoothLocalDevice localDevice;
    QVERIFY(localDevice.isValid());
}
void tst_QBluetoothLocalDevice::tst_allDevices()
{
    // we should have one local bluetooth device
    QVERIFY(QBluetoothLocalDevice::allDevices().count() == 1);
}
void tst_QBluetoothLocalDevice::tst_construction()
{
    QBluetoothLocalDevice localDevice;
    QVERIFY(localDevice.isValid());

    QBluetoothLocalDevice anotherDevice(QBluetoothAddress(000000000000));
    QVERIFY(anotherDevice.isValid());
    QVERIFY(anotherDevice.address().toUInt64() != 0);

}

void tst_QBluetoothLocalDevice::tst_pairDevice()
{
    QFETCH(QBluetoothAddress, deviceAddress);
    QFETCH(QBluetoothLocalDevice::Pairing, pairingExpected);

    qDebug() << "tst_pairDevice(): address=" << deviceAddress.toString() << "pairingModeExpected="
            << static_cast<int>(pairingExpected);

    QBluetoothLocalDevice localDevice;
    //powerOn if not already
    localDevice.powerOn();

    QSignalSpy pairingSpy(&localDevice, SIGNAL(pairingFinished(const QBluetoothAddress &,QBluetoothLocalDevice::Pairing)) );
    // there should be no signals yet
    QVERIFY(pairingSpy.isEmpty());

    localDevice.requestPairing(deviceAddress, pairingExpected);
    // async, wait for it
    WAIT_FOR_CONDITION(pairingSpy.count(),1);
    QVERIFY(pairingSpy.count() > 0);

    // test the actual signal values.
    QList<QVariant> arguments = pairingSpy.takeFirst();
    QBluetoothAddress address = qvariant_cast<QBluetoothAddress>(arguments.at(0));
    QBluetoothLocalDevice::Pairing pairingResult = qvariant_cast<QBluetoothLocalDevice::Pairing>(arguments.at(1));
    QCOMPARE(deviceAddress, address);
    QCOMPARE(pairingExpected, pairingResult);

    QCOMPARE(pairingExpected, localDevice.pairingStatus(deviceAddress));

}

void tst_QBluetoothLocalDevice::tst_pairingStatus()
{
    QFETCH(QBluetoothAddress, deviceAddress);
    QFETCH(QBluetoothLocalDevice::Pairing, pairingExpected);

    qDebug() << "tst_pairingStatus(): address=" << deviceAddress.toString() << "pairingModeExpected="
            << static_cast<int>(pairingExpected);

    QBluetoothLocalDevice localDevice;
    QCOMPARE(pairingExpected, localDevice.pairingStatus(deviceAddress));
}
QTEST_MAIN(tst_QBluetoothLocalDevice)

#include "tst_qbluetoothlocaldevice.moc"
