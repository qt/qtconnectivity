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

#include <qbluetoothaddress.h>
#include <qbluetoothdeviceinfo.h>
#include <qbluetoothlocaldevice.h>
#include <qbluetoothuuid.h>

Q_DECLARE_METATYPE(QBluetoothDeviceInfo::ServiceClasses)
Q_DECLARE_METATYPE(QBluetoothDeviceInfo::MajorDeviceClass)

class tst_QBluetoothDeviceInfo : public QObject
{
    Q_OBJECT

public:
    tst_QBluetoothDeviceInfo();
    ~tst_QBluetoothDeviceInfo();

private slots:
    void initTestCase();

    void tst_construction_data();
    void tst_construction();

    void tst_assignment_data();
    void tst_assignment();

    void tst_serviceUuids();

    void tst_cached();

    void tst_manufacturerSpecificData();
};

tst_QBluetoothDeviceInfo::tst_QBluetoothDeviceInfo()
{
}

tst_QBluetoothDeviceInfo::~tst_QBluetoothDeviceInfo()
{
}

void tst_QBluetoothDeviceInfo::initTestCase()
{
    qRegisterMetaType<QBluetoothDeviceInfo::ServiceClasses>("QBluetoothDeviceInfo::ServiceClasses");
    qRegisterMetaType<QBluetoothDeviceInfo::MajorDeviceClass>("QBluetoothDeviceInfo::MajorDeviceClass");
    // start Bluetooth if not started
    QBluetoothLocalDevice *device = new QBluetoothLocalDevice();
    device->powerOn();
    delete device;
}

void tst_QBluetoothDeviceInfo::tst_construction_data()
{
    QTest::addColumn<QBluetoothAddress>("address");
    QTest::addColumn<QString>("name");
    QTest::addColumn<quint32>("classOfDevice");
    QTest::addColumn<QBluetoothDeviceInfo::ServiceClasses>("serviceClasses");
    QTest::addColumn<QBluetoothDeviceInfo::MajorDeviceClass>("majorDeviceClass");
    QTest::addColumn<quint8>("minorDeviceClass");

    // bits 12-8 Major
    // bits 7-2 Minor
    // bits 1-0 0

    QTest::newRow("0x000000 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x000000)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::NoService)
        << QBluetoothDeviceInfo::MiscellaneousDevice
        << quint8(QBluetoothDeviceInfo::UncategorizedMiscellaneous);
    QTest::newRow("0x000100 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x000100)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::NoService)
        << QBluetoothDeviceInfo::ComputerDevice
        << quint8(QBluetoothDeviceInfo::UncategorizedComputer);
    QTest::newRow("0x000104 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x000104)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::NoService)
        << QBluetoothDeviceInfo::ComputerDevice
        << quint8(QBluetoothDeviceInfo::DesktopComputer);
    QTest::newRow("0x000118 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x000118)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::NoService)
        << QBluetoothDeviceInfo::ComputerDevice
        << quint8(QBluetoothDeviceInfo::WearableComputer);
    QTest::newRow("0x000200 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device" << quint32(0x000200)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::NoService)
        << QBluetoothDeviceInfo::PhoneDevice
        << quint8(QBluetoothDeviceInfo::UncategorizedPhone);
    QTest::newRow("0x000204 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x000204)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::NoService)
        << QBluetoothDeviceInfo::PhoneDevice
        << quint8(QBluetoothDeviceInfo::CellularPhone);
    QTest::newRow("0x000214 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device" << quint32(0x000214)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::NoService)
        << QBluetoothDeviceInfo::PhoneDevice
        << quint8(QBluetoothDeviceInfo::CommonIsdnAccessPhone);
    QTest::newRow("0x000300 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x000300)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::NoService)
        << QBluetoothDeviceInfo::LANAccessDevice
        << quint8(QBluetoothDeviceInfo::NetworkFullService);
    QTest::newRow("0x000320 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x000320)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::NoService)
        << QBluetoothDeviceInfo::LANAccessDevice
        << quint8(QBluetoothDeviceInfo::NetworkLoadFactorOne);
    QTest::newRow("0x0003E0 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x0003E0)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::NoService)
        << QBluetoothDeviceInfo::LANAccessDevice
        << quint8(QBluetoothDeviceInfo::NetworkNoService);
    QTest::newRow("0x000400 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x000400)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::NoService)
        << QBluetoothDeviceInfo::AudioVideoDevice
        << quint8(QBluetoothDeviceInfo::UncategorizedAudioVideoDevice);
    QTest::newRow("0x00044C COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x00044C)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::NoService)
        << QBluetoothDeviceInfo::AudioVideoDevice
        << quint8(QBluetoothDeviceInfo::GamingDevice);
    QTest::newRow("0x000500 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x000500)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::NoService)
        << QBluetoothDeviceInfo::PeripheralDevice
        << quint8(QBluetoothDeviceInfo::UncategorizedPeripheral);
    QTest::newRow("0x0005D8 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x0005D8)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::NoService)
        << QBluetoothDeviceInfo::PeripheralDevice
        << quint8(QBluetoothDeviceInfo::KeyboardWithPointingDevicePeripheral | QBluetoothDeviceInfo::CardReaderPeripheral);
    QTest::newRow("0x000600 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x000600)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::NoService)
        << QBluetoothDeviceInfo::ImagingDevice
        << quint8(QBluetoothDeviceInfo::UncategorizedImagingDevice);
    QTest::newRow("0x000680 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x000680)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::NoService)
        << QBluetoothDeviceInfo::ImagingDevice
        << quint8(QBluetoothDeviceInfo::ImagePrinter);
    QTest::newRow("0x000700 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x000700)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::NoService)
        << QBluetoothDeviceInfo::WearableDevice
        << quint8(QBluetoothDeviceInfo::UncategorizedWearableDevice);
    QTest::newRow("0x000714 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x000714)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::NoService)
        << QBluetoothDeviceInfo::WearableDevice
        << quint8(QBluetoothDeviceInfo::WearableGlasses);
    QTest::newRow("0x000800 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x000800)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::NoService)
        << QBluetoothDeviceInfo::ToyDevice
        << quint8(QBluetoothDeviceInfo::UncategorizedToy);
    QTest::newRow("0x000814 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x000814)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::NoService)
        << QBluetoothDeviceInfo::ToyDevice
        << quint8(QBluetoothDeviceInfo::ToyGame);
    QTest::newRow("0x001f00 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x001f00)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::NoService)
        << QBluetoothDeviceInfo::UncategorizedDevice
        << quint8(0);
    QTest::newRow("0x002000 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x002000)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::PositioningService)
        << QBluetoothDeviceInfo::MiscellaneousDevice
        << quint8(QBluetoothDeviceInfo::UncategorizedMiscellaneous);
    QTest::newRow("0x100000 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x100000)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::InformationService)
        << QBluetoothDeviceInfo::MiscellaneousDevice
        << quint8(QBluetoothDeviceInfo::UncategorizedMiscellaneous);
    QTest::newRow("0xFFE000 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0xFFE000)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::AllServices)
        << QBluetoothDeviceInfo::MiscellaneousDevice
        << quint8(QBluetoothDeviceInfo::UncategorizedMiscellaneous);
}

void tst_QBluetoothDeviceInfo::tst_construction()
{
    {
        QBluetoothDeviceInfo deviceInfo;

        QVERIFY(!deviceInfo.isValid());
    }

    {
        QFETCH(QBluetoothAddress, address);
        QFETCH(QString, name);
        QFETCH(quint32, classOfDevice);
        QFETCH(QBluetoothDeviceInfo::ServiceClasses, serviceClasses);
        QFETCH(QBluetoothDeviceInfo::MajorDeviceClass, majorDeviceClass);
        QFETCH(quint8, minorDeviceClass);

        QBluetoothDeviceInfo deviceInfo(address, name, classOfDevice);

        QVERIFY(deviceInfo.isValid());

        QCOMPARE(deviceInfo.address(), address);
        QCOMPARE(deviceInfo.name(), name);
        QCOMPARE(deviceInfo.serviceClasses(), serviceClasses);
        QCOMPARE(deviceInfo.majorDeviceClass(), majorDeviceClass);
        QCOMPARE(deviceInfo.minorDeviceClass(), minorDeviceClass);

        QBluetoothDeviceInfo copyInfo(deviceInfo);

        QVERIFY(copyInfo.isValid());

        QCOMPARE(copyInfo.address(), address);
        QCOMPARE(copyInfo.name(), name);
        QCOMPARE(copyInfo.serviceClasses(), serviceClasses);
        QCOMPARE(copyInfo.majorDeviceClass(), majorDeviceClass);
        QCOMPARE(copyInfo.minorDeviceClass(), minorDeviceClass);
    }
}

void tst_QBluetoothDeviceInfo::tst_assignment_data()
{
    tst_construction_data();
}

void tst_QBluetoothDeviceInfo::tst_assignment()
{
    QFETCH(QBluetoothAddress, address);
    QFETCH(QString, name);
    QFETCH(quint32, classOfDevice);
    QFETCH(QBluetoothDeviceInfo::ServiceClasses, serviceClasses);
    QFETCH(QBluetoothDeviceInfo::MajorDeviceClass, majorDeviceClass);
    QFETCH(quint8, minorDeviceClass);

    QBluetoothDeviceInfo deviceInfo(address, name, classOfDevice);

    QVERIFY(deviceInfo.isValid());

    {
        QBluetoothDeviceInfo copyInfo = deviceInfo;

        QVERIFY(copyInfo.isValid());

        QCOMPARE(copyInfo.address(), address);
        QCOMPARE(copyInfo.name(), name);
        QCOMPARE(copyInfo.serviceClasses(), serviceClasses);
        QCOMPARE(copyInfo.majorDeviceClass(), majorDeviceClass);
        QCOMPARE(copyInfo.minorDeviceClass(), minorDeviceClass);
    }

    {
        QBluetoothDeviceInfo copyInfo;

        QVERIFY(!copyInfo.isValid());

        copyInfo = deviceInfo;

        QVERIFY(copyInfo.isValid());

        QCOMPARE(copyInfo.address(), address);
        QCOMPARE(copyInfo.name(), name);
        QCOMPARE(copyInfo.serviceClasses(), serviceClasses);
        QCOMPARE(copyInfo.majorDeviceClass(), majorDeviceClass);
        QCOMPARE(copyInfo.minorDeviceClass(), minorDeviceClass);
    }

    {
        QBluetoothDeviceInfo copyInfo1;
        QBluetoothDeviceInfo copyInfo2;

        QVERIFY(!copyInfo1.isValid());
        QVERIFY(!copyInfo2.isValid());

        copyInfo1 = copyInfo2 = deviceInfo;

        QVERIFY(copyInfo1.isValid());
        QVERIFY(copyInfo2.isValid());
        QVERIFY(!(QBluetoothDeviceInfo() == copyInfo1));

        QCOMPARE(copyInfo1.address(), address);
        QCOMPARE(copyInfo2.address(), address);
        QCOMPARE(copyInfo1.name(), name);
        QCOMPARE(copyInfo2.name(), name);
        QCOMPARE(copyInfo1.serviceClasses(), serviceClasses);
        QCOMPARE(copyInfo2.serviceClasses(), serviceClasses);
        QCOMPARE(copyInfo1.majorDeviceClass(), majorDeviceClass);
        QCOMPARE(copyInfo2.majorDeviceClass(), majorDeviceClass);
        QCOMPARE(copyInfo1.minorDeviceClass(), minorDeviceClass);
        QCOMPARE(copyInfo2.minorDeviceClass(), minorDeviceClass);
    }

    {
        QBluetoothDeviceInfo testDeviceInfo;
        QVERIFY(testDeviceInfo == QBluetoothDeviceInfo());
    }
}

void tst_QBluetoothDeviceInfo::tst_serviceUuids()
{
    QBluetoothDeviceInfo deviceInfo;
    QBluetoothDeviceInfo copyInfo = deviceInfo;

    QList<QBluetoothUuid> servicesList;
    servicesList.append(QBluetoothUuid::L2cap);
    servicesList.append(QBluetoothUuid::Rfcomm);
    QVERIFY(servicesList.count() > 0);

    deviceInfo.setServiceUuids(servicesList, QBluetoothDeviceInfo::DataComplete);
    QVERIFY(deviceInfo.serviceUuids().count() > 0);
    QVERIFY(!(deviceInfo == copyInfo));

    QVERIFY(deviceInfo.serviceUuidsCompleteness() == QBluetoothDeviceInfo::DataComplete);
}

void tst_QBluetoothDeviceInfo::tst_cached()
{
    QBluetoothDeviceInfo deviceInfo(QBluetoothAddress("AABBCCDDEEFF"),
        QString("My Bluetooth Device"), quint32(0x002000));
    QBluetoothDeviceInfo copyInfo = deviceInfo;

    QVERIFY(!deviceInfo.isCached());
    deviceInfo.setCached(true);
    QVERIFY(deviceInfo.isCached());
    QVERIFY(!(deviceInfo == copyInfo));

    deviceInfo.setCached(false);
    QVERIFY(!(deviceInfo.isCached()));
}

void tst_QBluetoothDeviceInfo::tst_manufacturerSpecificData()
{
    QBluetoothDeviceInfo deviceInfo;
    QByteArray data;
    bool available;
    data = deviceInfo.manufacturerSpecificData(&available);
    // Current API implementation returns only empty QByteArray()
    QCOMPARE(data, QByteArray());
}

QTEST_MAIN(tst_QBluetoothDeviceInfo)

#include "tst_qbluetoothdeviceinfo.moc"
