/***************************************************************************
**
** Copyright (C) 2013 BlackBerry Limited all rights reserved
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia. For licensing terms and
** conditions see http://qt.digia.com/licensing. For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights. These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QUuid>

#include <QDebug>

#include <qbluetoothdeviceinfo.h>
#include <qlowenergyserviceinfo.h>
#include <qbluetoothaddress.h>
#include <qbluetoothlocaldevice.h>
#include <qbluetoothuuid.h>

QT_USE_NAMESPACE

Q_DECLARE_METATYPE(QUuid)
Q_DECLARE_METATYPE(QBluetoothDeviceInfo::CoreConfiguration)
Q_DECLARE_METATYPE(QLowEnergyServiceInfo)
Q_DECLARE_METATYPE(QBluetoothUuid::ServiceClassUuid)

class tst_QLowEnergyServiceInfo : public QObject
{
    Q_OBJECT

public:
    tst_QLowEnergyServiceInfo();
    ~tst_QLowEnergyServiceInfo();

private slots:
    void initTestCase();
    void tst_construction();
    void tst_assignment_data();
    void tst_assignment();
};

tst_QLowEnergyServiceInfo::tst_QLowEnergyServiceInfo()
{
}

tst_QLowEnergyServiceInfo::~tst_QLowEnergyServiceInfo()
{
}

void tst_QLowEnergyServiceInfo::initTestCase()
{
    // start Bluetooth if not started
    QBluetoothLocalDevice *device = new QBluetoothLocalDevice();
    device->powerOn();
    delete device;
}

void tst_QLowEnergyServiceInfo::tst_construction()
{
    const QBluetoothUuid serviceUuid(QBluetoothUuid::HeartRate);
    const QBluetoothUuid alternateServiceUuid(QBluetoothUuid::BatteryService);
    const QBluetoothDeviceInfo deviceInfo(QBluetoothAddress("001122334455"), "Test Device", 0);
    const QBluetoothDeviceInfo alternatedeviceInfo(QBluetoothAddress("554433221100"), "Test Device2", 0);

    {
        QLowEnergyServiceInfo serviceInfo;

        QVERIFY(!serviceInfo.isValid());
        QCOMPARE(serviceInfo.serviceName(), QStringLiteral("Unknown Service"));
        QCOMPARE(serviceInfo.serviceUuid().toString(), QBluetoothUuid().toString());
        QCOMPARE(serviceInfo.device(), QBluetoothDeviceInfo());
    }

    {
        QLowEnergyServiceInfo serviceInfo(serviceUuid);
        serviceInfo.setDevice(deviceInfo);

        QVERIFY(serviceInfo.isValid());

        QCOMPARE(serviceInfo.serviceUuid().toString(), serviceUuid.toString());
        QCOMPARE(serviceInfo.device().address(), deviceInfo.address());

        QLowEnergyServiceInfo copyInfo(serviceInfo);

        QVERIFY(copyInfo.isValid());

        QCOMPARE(copyInfo.serviceUuid().toString(), serviceUuid.toString());
        QCOMPARE(copyInfo.device().address(), deviceInfo.address());


        copyInfo = QLowEnergyServiceInfo(alternateServiceUuid);
        copyInfo.setDevice(alternatedeviceInfo);
        QCOMPARE(copyInfo.serviceUuid(), alternateServiceUuid);

        QCOMPARE(copyInfo.device().address(), alternatedeviceInfo.address());

    }
}

void tst_QLowEnergyServiceInfo::tst_assignment_data()
{
    QTest::addColumn<QBluetoothAddress>("address");
    QTest::addColumn<QString>("name");
    QTest::addColumn<quint32>("classOfDevice");
    QTest::addColumn<QBluetoothUuid>("serviceClassUuid");
    QTest::addColumn<QBluetoothDeviceInfo::CoreConfiguration>("coreConfiguration");

    // bits 12-8 Major
    // bits 7-2 Minor
    // bits 1-0 0

    QTest::newRow("0x000000 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x000000)
        << QBluetoothUuid(QBluetoothUuid::GenericAccess)
        << QBluetoothDeviceInfo::BaseRateAndLowEnergyCoreConfiguration;
    QTest::newRow("0x000100 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x000100)
        << QBluetoothUuid(QBluetoothUuid::GenericAttribute)
        << QBluetoothDeviceInfo::BaseRateAndLowEnergyCoreConfiguration;
    QTest::newRow("0x000104 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x000104)
        << QBluetoothUuid(QBluetoothUuid::HeartRate)
        << QBluetoothDeviceInfo::BaseRateAndLowEnergyCoreConfiguration;
    QTest::newRow("0x000118 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x000118)
        << QBluetoothUuid(QBluetoothUuid::CyclingSpeedAndCadence)
        << QBluetoothDeviceInfo::BaseRateAndLowEnergyCoreConfiguration;
    QTest::newRow("0x000200 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x000200)
        << QBluetoothUuid(QBluetoothUuid::CyclingPower)
        << QBluetoothDeviceInfo::LowEnergyCoreConfiguration;
    QTest::newRow("0x000204 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x000204)
        << QBluetoothUuid(QBluetoothUuid::ScanParameters)
        << QBluetoothDeviceInfo::LowEnergyCoreConfiguration;
    QTest::newRow("0x000214 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x000214)
        << QBluetoothUuid(QBluetoothUuid::DeviceInformation)
        << QBluetoothDeviceInfo::LowEnergyCoreConfiguration;
    QTest::newRow("0x000300 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x000300)
        << QBluetoothUuid(QBluetoothUuid::CurrentTimeService)
        << QBluetoothDeviceInfo::LowEnergyCoreConfiguration;
    QTest::newRow("0x000320 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x000320)
        << QBluetoothUuid(QBluetoothUuid::LocationAndNavigation)
        << QBluetoothDeviceInfo::LowEnergyCoreConfiguration;
}

void tst_QLowEnergyServiceInfo::tst_assignment()
{
    QFETCH(QBluetoothAddress, address);
    QFETCH(QString, name);
    QFETCH(quint32, classOfDevice);
    QFETCH(QBluetoothUuid, serviceClassUuid);
    QFETCH(QBluetoothDeviceInfo::CoreConfiguration, coreConfiguration);

    QBluetoothDeviceInfo deviceInfo(address, name, classOfDevice);
    deviceInfo.setCoreConfigurations(coreConfiguration);
    QCOMPARE(deviceInfo.coreConfigurations(), coreConfiguration);

    QLowEnergyServiceInfo serviceInfo(serviceClassUuid);
    serviceInfo.setDevice(deviceInfo);
    QCOMPARE(serviceInfo.device(), deviceInfo);

    QVERIFY(serviceInfo.isValid());

    {
        QLowEnergyServiceInfo copyInfo = serviceInfo;

        QVERIFY(copyInfo.isValid());

        QCOMPARE(copyInfo.device().address(), address);
        QCOMPARE(copyInfo.serviceUuid(), serviceClassUuid);
        QCOMPARE(copyInfo.device().coreConfigurations(), coreConfiguration);
        QCOMPARE(copyInfo.device(), deviceInfo);
    }

    {
        QLowEnergyServiceInfo copyInfo;

        QVERIFY(!copyInfo.isValid());

        copyInfo = serviceInfo;

        QVERIFY(copyInfo.isValid());

        QCOMPARE(copyInfo.device().address(), address);
        QCOMPARE(copyInfo.serviceUuid(), serviceClassUuid);
        QCOMPARE(copyInfo.device().coreConfigurations(), coreConfiguration);
    }

    {
        QLowEnergyServiceInfo copyInfo1;
        QLowEnergyServiceInfo copyInfo2;

        QVERIFY(!copyInfo1.isValid());
        QVERIFY(!copyInfo2.isValid());

        copyInfo1 = copyInfo2 = serviceInfo;

        QVERIFY(copyInfo1.isValid());
        QVERIFY(copyInfo2.isValid());

        QCOMPARE(copyInfo1.device().address(), address);
        QCOMPARE(copyInfo2.device().address(), address);
        QCOMPARE(copyInfo1.serviceUuid(), serviceClassUuid);
        QCOMPARE(copyInfo2.serviceUuid(), serviceClassUuid);
        QCOMPARE(copyInfo1.device().coreConfigurations(), coreConfiguration);
        QCOMPARE(copyInfo2.device().coreConfigurations(), coreConfiguration);
        QCOMPARE(copyInfo1.device(), deviceInfo);
        QCOMPARE(copyInfo2.device(), deviceInfo);
    }
}

QTEST_MAIN(tst_QLowEnergyServiceInfo)

#include "tst_qlowenergyserviceinfo.moc"
