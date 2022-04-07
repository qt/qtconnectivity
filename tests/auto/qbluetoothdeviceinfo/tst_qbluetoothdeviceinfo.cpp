/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtTest/QtTest>

#include <QScopedPointer>
#include <QDebug>

#include <qbluetoothaddress.h>
#include <qbluetoothdeviceinfo.h>
#include <qbluetoothlocaldevice.h>
#include <qbluetoothuuid.h>

QT_USE_NAMESPACE

Q_DECLARE_METATYPE(QBluetoothDeviceInfo::ServiceClasses)
Q_DECLARE_METATYPE(QBluetoothDeviceInfo::MajorDeviceClass)
Q_DECLARE_METATYPE(QBluetoothDeviceInfo::CoreConfiguration)

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

    void tst_flags();

    void tst_manufacturerData();
};

tst_QBluetoothDeviceInfo::tst_QBluetoothDeviceInfo()
{
}

tst_QBluetoothDeviceInfo::~tst_QBluetoothDeviceInfo()
{
}

void tst_QBluetoothDeviceInfo::initTestCase()
{
    qRegisterMetaType<QBluetoothDeviceInfo::ServiceClasses>();
    qRegisterMetaType<QBluetoothDeviceInfo::MajorDeviceClass>();
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
    QTest::addColumn<QBluetoothDeviceInfo::CoreConfiguration>("coreConfiguration");
    // On OS X and iOS there are no real addresses
    // with Core Bluetooth, only 'uuids' (128-bit) generated by Apple instead.
    QTest::addColumn<QBluetoothUuid>("deviceUuid");
    const QBluetoothUuid leDeviceUuid(QString("6C903349-31E2-40EF-826B-1E62C0D884E2"));

    // bits 12-8 Major
    // bits 7-2 Minor
    // bits 1-0 0

    QTest::newRow("0x000000 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x000000)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::NoService)
        << QBluetoothDeviceInfo::MiscellaneousDevice
        << quint8(QBluetoothDeviceInfo::UncategorizedMiscellaneous)
        << QBluetoothDeviceInfo::BaseRateCoreConfiguration
        << leDeviceUuid;
    QTest::newRow("0x000100 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x000100)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::NoService)
        << QBluetoothDeviceInfo::ComputerDevice
        << quint8(QBluetoothDeviceInfo::UncategorizedComputer)
        << QBluetoothDeviceInfo::BaseRateCoreConfiguration
        << leDeviceUuid;
    QTest::newRow("0x000104 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x000104)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::NoService)
        << QBluetoothDeviceInfo::ComputerDevice
        << quint8(QBluetoothDeviceInfo::DesktopComputer)
        << QBluetoothDeviceInfo::BaseRateAndLowEnergyCoreConfiguration
        << leDeviceUuid;
    QTest::newRow("0x000118 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x000118)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::NoService)
        << QBluetoothDeviceInfo::ComputerDevice
        << quint8(QBluetoothDeviceInfo::WearableComputer)
        << QBluetoothDeviceInfo::BaseRateAndLowEnergyCoreConfiguration
        << leDeviceUuid;
    QTest::newRow("0x000200 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device" << quint32(0x000200)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::NoService)
        << QBluetoothDeviceInfo::PhoneDevice
        << quint8(QBluetoothDeviceInfo::UncategorizedPhone)
        << QBluetoothDeviceInfo::BaseRateAndLowEnergyCoreConfiguration
        << leDeviceUuid;
    QTest::newRow("0x000204 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x000204)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::NoService)
        << QBluetoothDeviceInfo::PhoneDevice
        << quint8(QBluetoothDeviceInfo::CellularPhone)
        << QBluetoothDeviceInfo::BaseRateAndLowEnergyCoreConfiguration
        << leDeviceUuid;
    QTest::newRow("0x000214 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device" << quint32(0x000214)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::NoService)
        << QBluetoothDeviceInfo::PhoneDevice
        << quint8(QBluetoothDeviceInfo::CommonIsdnAccessPhone)
        << QBluetoothDeviceInfo::BaseRateAndLowEnergyCoreConfiguration
        << leDeviceUuid;
    QTest::newRow("0x000300 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x000300)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::NoService)
        << QBluetoothDeviceInfo::NetworkDevice
        << quint8(QBluetoothDeviceInfo::NetworkFullService)
        << QBluetoothDeviceInfo::BaseRateAndLowEnergyCoreConfiguration
        << leDeviceUuid;
    QTest::newRow("0x000320 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x000320)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::NoService)
        << QBluetoothDeviceInfo::NetworkDevice
        << quint8(QBluetoothDeviceInfo::NetworkLoadFactorOne)
        << QBluetoothDeviceInfo::BaseRateAndLowEnergyCoreConfiguration
        << leDeviceUuid;
    QTest::newRow("0x0003E0 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x0003E0)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::NoService)
        << QBluetoothDeviceInfo::NetworkDevice
        << quint8(QBluetoothDeviceInfo::NetworkNoService)
        << QBluetoothDeviceInfo::BaseRateAndLowEnergyCoreConfiguration
        << leDeviceUuid;
    QTest::newRow("0x000400 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x000400)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::NoService)
        << QBluetoothDeviceInfo::AudioVideoDevice
        << quint8(QBluetoothDeviceInfo::UncategorizedAudioVideoDevice)
        << QBluetoothDeviceInfo::BaseRateAndLowEnergyCoreConfiguration
        << leDeviceUuid;
    QTest::newRow("0x000448 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x000448)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::NoService)
        << QBluetoothDeviceInfo::AudioVideoDevice
        << quint8(QBluetoothDeviceInfo::GamingDevice)
        << QBluetoothDeviceInfo::BaseRateAndLowEnergyCoreConfiguration
        << leDeviceUuid;
    QTest::newRow("0x000500 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x000500)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::NoService)
        << QBluetoothDeviceInfo::PeripheralDevice
        << quint8(QBluetoothDeviceInfo::UncategorizedPeripheral)
        << QBluetoothDeviceInfo::BaseRateAndLowEnergyCoreConfiguration
        << leDeviceUuid;
    QTest::newRow("0x0005D8 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x0005D8)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::NoService)
        << QBluetoothDeviceInfo::PeripheralDevice
        << quint8(QBluetoothDeviceInfo::KeyboardWithPointingDevicePeripheral | QBluetoothDeviceInfo::CardReaderPeripheral)
        << QBluetoothDeviceInfo::BaseRateAndLowEnergyCoreConfiguration
        << leDeviceUuid;
    QTest::newRow("0x000600 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x000600)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::NoService)
        << QBluetoothDeviceInfo::ImagingDevice
        << quint8(QBluetoothDeviceInfo::UncategorizedImagingDevice)
        << QBluetoothDeviceInfo::BaseRateAndLowEnergyCoreConfiguration
        << leDeviceUuid;
    QTest::newRow("0x000680 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x000680)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::NoService)
        << QBluetoothDeviceInfo::ImagingDevice
        << quint8(QBluetoothDeviceInfo::ImagePrinter)
        << QBluetoothDeviceInfo::LowEnergyCoreConfiguration
        << leDeviceUuid;
    QTest::newRow("0x000700 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x000700)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::NoService)
        << QBluetoothDeviceInfo::WearableDevice
        << quint8(QBluetoothDeviceInfo::UncategorizedWearableDevice)
        << QBluetoothDeviceInfo::LowEnergyCoreConfiguration
        << leDeviceUuid;
    QTest::newRow("0x000714 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x000714)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::NoService)
        << QBluetoothDeviceInfo::WearableDevice
        << quint8(QBluetoothDeviceInfo::WearableGlasses)
        << QBluetoothDeviceInfo::LowEnergyCoreConfiguration
        << leDeviceUuid;
    QTest::newRow("0x000800 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x000800)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::NoService)
        << QBluetoothDeviceInfo::ToyDevice
        << quint8(QBluetoothDeviceInfo::UncategorizedToy)
        << QBluetoothDeviceInfo::LowEnergyCoreConfiguration
        << leDeviceUuid;
    QTest::newRow("0x000814 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x000814)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::NoService)
        << QBluetoothDeviceInfo::ToyDevice
        << quint8(QBluetoothDeviceInfo::ToyGame)
        << QBluetoothDeviceInfo::LowEnergyCoreConfiguration
        << leDeviceUuid;
    QTest::newRow("0x001f00 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x001f00)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::NoService)
        << QBluetoothDeviceInfo::UncategorizedDevice
        << quint8(0)
        << QBluetoothDeviceInfo::LowEnergyCoreConfiguration
        << leDeviceUuid;
    QTest::newRow("0x002000 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x002000)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::PositioningService)
        << QBluetoothDeviceInfo::MiscellaneousDevice
        << quint8(QBluetoothDeviceInfo::UncategorizedMiscellaneous)
        << QBluetoothDeviceInfo::LowEnergyCoreConfiguration
        << leDeviceUuid;
    QTest::newRow("0x100000 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0x100000)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::InformationService)
        << QBluetoothDeviceInfo::MiscellaneousDevice
        << quint8(QBluetoothDeviceInfo::UncategorizedMiscellaneous)
        << QBluetoothDeviceInfo::LowEnergyCoreConfiguration
        << leDeviceUuid;
    QTest::newRow("0xFFE000 COD") << QBluetoothAddress("000000000000") << "My Bluetooth Device"
        << quint32(0xFFE000)
        << QBluetoothDeviceInfo::ServiceClasses(QBluetoothDeviceInfo::AllServices)
        << QBluetoothDeviceInfo::MiscellaneousDevice
        << quint8(QBluetoothDeviceInfo::UncategorizedMiscellaneous)
        << QBluetoothDeviceInfo::LowEnergyCoreConfiguration
        << leDeviceUuid;
}

void tst_QBluetoothDeviceInfo::tst_construction()
{
    {
        QBluetoothDeviceInfo deviceInfo;

        QVERIFY(!deviceInfo.isValid());
        QVERIFY(deviceInfo.coreConfigurations()
                    == QBluetoothDeviceInfo::UnknownCoreConfiguration);
    }

    {
        QFETCH(QBluetoothAddress, address);
        QFETCH(QString, name);
        QFETCH(quint32, classOfDevice);
        QFETCH(QBluetoothDeviceInfo::ServiceClasses, serviceClasses);
        QFETCH(QBluetoothDeviceInfo::MajorDeviceClass, majorDeviceClass);
        QFETCH(quint8, minorDeviceClass);
        QFETCH(QBluetoothDeviceInfo::CoreConfiguration, coreConfiguration);
        QFETCH(QBluetoothUuid, deviceUuid);

        QBluetoothDeviceInfo deviceInfo(address, name, classOfDevice);
        QVERIFY(deviceInfo.isValid());

        QCOMPARE(deviceInfo.address(), address);
        QCOMPARE(deviceInfo.name(), name);
        QCOMPARE(deviceInfo.serviceClasses(), serviceClasses);
        QCOMPARE(deviceInfo.majorDeviceClass(), majorDeviceClass);
        QCOMPARE(deviceInfo.minorDeviceClass(), minorDeviceClass);
        QCOMPARE(deviceInfo.coreConfigurations(), QBluetoothDeviceInfo::UnknownCoreConfiguration);

        deviceInfo.setCoreConfigurations(coreConfiguration);
        QCOMPARE(deviceInfo.coreConfigurations(), coreConfiguration);

        deviceInfo.setDeviceUuid(deviceUuid);
        QCOMPARE(deviceInfo.deviceUuid(), deviceUuid);

        QBluetoothDeviceInfo copyInfo(deviceInfo);
        QVERIFY(copyInfo.isValid());

        QCOMPARE(copyInfo.address(), address);
        QCOMPARE(copyInfo.name(), name);
        QCOMPARE(copyInfo.serviceClasses(), serviceClasses);
        QCOMPARE(copyInfo.majorDeviceClass(), majorDeviceClass);
        QCOMPARE(copyInfo.minorDeviceClass(), minorDeviceClass);
        QCOMPARE(copyInfo.coreConfigurations(), coreConfiguration);
        QCOMPARE(copyInfo.deviceUuid(), deviceUuid);
    }

    {
        // Test construction from the device unique UUID, without an address.
        QFETCH(QString, name);
        QFETCH(quint32, classOfDevice);
        QFETCH(QBluetoothDeviceInfo::ServiceClasses, serviceClasses);
        QFETCH(QBluetoothDeviceInfo::MajorDeviceClass, majorDeviceClass);
        QFETCH(quint8, minorDeviceClass);
        QFETCH(QBluetoothDeviceInfo::CoreConfiguration, coreConfiguration);
        QFETCH(QBluetoothUuid, deviceUuid);

        QBluetoothDeviceInfo deviceInfo(deviceUuid, name, classOfDevice);
        QVERIFY(deviceInfo.isValid());

        QCOMPARE(deviceInfo.name(), name);
        QCOMPARE(deviceInfo.serviceClasses(), serviceClasses);
        QCOMPARE(deviceInfo.majorDeviceClass(), majorDeviceClass);
        QCOMPARE(deviceInfo.minorDeviceClass(), minorDeviceClass);
        QCOMPARE(deviceInfo.coreConfigurations(), QBluetoothDeviceInfo::UnknownCoreConfiguration);

        deviceInfo.setCoreConfigurations(coreConfiguration);
        QCOMPARE(deviceInfo.coreConfigurations(), coreConfiguration);

        QBluetoothDeviceInfo copyInfo(deviceInfo);
        QVERIFY(copyInfo.isValid());

        QCOMPARE(copyInfo.name(), name);
        QCOMPARE(copyInfo.serviceClasses(), serviceClasses);
        QCOMPARE(copyInfo.majorDeviceClass(), majorDeviceClass);
        QCOMPARE(copyInfo.minorDeviceClass(), minorDeviceClass);
        QCOMPARE(copyInfo.coreConfigurations(), coreConfiguration);
        QCOMPARE(copyInfo.deviceUuid(), deviceUuid);
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
    QFETCH(QBluetoothDeviceInfo::CoreConfiguration, coreConfiguration);
    QFETCH(QBluetoothUuid, deviceUuid);

    QBluetoothDeviceInfo deviceInfo(address, name, classOfDevice);

    deviceInfo.setDeviceUuid(deviceUuid);
    deviceInfo.setCoreConfigurations(coreConfiguration);

    QVERIFY(deviceInfo.isValid());

    {
        QBluetoothDeviceInfo copyInfo = deviceInfo;

        QVERIFY(copyInfo.isValid());

        QCOMPARE(copyInfo.address(), address);
        QCOMPARE(copyInfo.name(), name);
        QCOMPARE(copyInfo.serviceClasses(), serviceClasses);
        QCOMPARE(copyInfo.majorDeviceClass(), majorDeviceClass);
        QCOMPARE(copyInfo.minorDeviceClass(), minorDeviceClass);
        QCOMPARE(copyInfo.coreConfigurations(), coreConfiguration);
        QCOMPARE(copyInfo.deviceUuid(), deviceUuid);
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
        QCOMPARE(copyInfo.coreConfigurations(), coreConfiguration);
        QCOMPARE(copyInfo.deviceUuid(), deviceUuid);
    }

    {
        QBluetoothDeviceInfo copyInfo1;
        QBluetoothDeviceInfo copyInfo2;

        QVERIFY(!copyInfo1.isValid());
        QVERIFY(!copyInfo2.isValid());

        copyInfo1 = copyInfo2 = deviceInfo;

        QVERIFY(copyInfo1.isValid());
        QVERIFY(copyInfo2.isValid());
        QVERIFY(QBluetoothDeviceInfo() != copyInfo1);

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
        QCOMPARE(copyInfo1.coreConfigurations(), coreConfiguration);
        QCOMPARE(copyInfo2.coreConfigurations(), coreConfiguration);
        QCOMPARE(copyInfo1.deviceUuid(), deviceUuid);
        QCOMPARE(copyInfo2.deviceUuid(), deviceUuid);
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
    servicesList.append(QBluetoothUuid::ProtocolUuid::L2cap);
    servicesList.append(QBluetoothUuid::ProtocolUuid::Rfcomm);
    QVERIFY(!servicesList.isEmpty());

    deviceInfo.setServiceUuids(servicesList);
    QVERIFY(!deviceInfo.serviceUuids().isEmpty());
    deviceInfo.setServiceUuids(QList<QBluetoothUuid>());
    QCOMPARE(deviceInfo.serviceUuids().size(), 0);
}

void tst_QBluetoothDeviceInfo::tst_cached()
{
    QBluetoothDeviceInfo deviceInfo(QBluetoothAddress("AABBCCDDEEFF"),
        QString("My Bluetooth Device"), quint32(0x002000));
    QBluetoothDeviceInfo copyInfo = deviceInfo;

    QVERIFY(!deviceInfo.isCached());
    deviceInfo.setCached(true);
    QVERIFY(deviceInfo.isCached());
    QVERIFY(deviceInfo != copyInfo);

    deviceInfo.setCached(false);
    QVERIFY(!(deviceInfo.isCached()));
}

void tst_QBluetoothDeviceInfo::tst_flags()
{
    QBluetoothDeviceInfo::CoreConfigurations flags1(QBluetoothDeviceInfo::LowEnergyCoreConfiguration);
    QBluetoothDeviceInfo::CoreConfigurations flags2(QBluetoothDeviceInfo::BaseRateCoreConfiguration);
    QBluetoothDeviceInfo::CoreConfigurations result;

    // test QFlags &operator|=(QFlags f)
    result = flags1 | flags2;
    QVERIFY(result.testFlag(QBluetoothDeviceInfo::LowEnergyCoreConfiguration));
    QVERIFY(result.testFlag(QBluetoothDeviceInfo::BaseRateCoreConfiguration));

    // test QFlags &operator|=(Enum f)
    result = flags1 | QBluetoothDeviceInfo::BaseRateCoreConfiguration;
    QVERIFY(result.testFlag(QBluetoothDeviceInfo::LowEnergyCoreConfiguration));
    QVERIFY(result.testFlag(QBluetoothDeviceInfo::BaseRateCoreConfiguration));

    // test Q_DECLARE_OPERATORS_FOR_FLAGS(QBluetoothDeviceInfo::CoreConfigurations)
    result = QBluetoothDeviceInfo::BaseRateCoreConfiguration | flags1;
    QVERIFY(result.testFlag(QBluetoothDeviceInfo::LowEnergyCoreConfiguration));
    QVERIFY(result.testFlag(QBluetoothDeviceInfo::BaseRateCoreConfiguration));

    QBluetoothDeviceInfo::ServiceClasses serviceFlag1(QBluetoothDeviceInfo::AudioService);
    QBluetoothDeviceInfo::ServiceClasses serviceFlag2(QBluetoothDeviceInfo::CapturingService);
    QBluetoothDeviceInfo::ServiceClasses serviceResult;

    // test QFlags &operator|=(QFlags f)
    serviceResult = serviceFlag1 | serviceFlag2;
    QVERIFY(serviceResult.testFlag(QBluetoothDeviceInfo::AudioService));
    QVERIFY(serviceResult.testFlag(QBluetoothDeviceInfo::CapturingService));

    // test QFlags &operator|=(Enum f)
    serviceResult = serviceFlag1 | QBluetoothDeviceInfo::CapturingService;
    QVERIFY(serviceResult.testFlag(QBluetoothDeviceInfo::AudioService));
    QVERIFY(serviceResult.testFlag(QBluetoothDeviceInfo::CapturingService));

    // test Q_DECLARE_OPERATORS_FOR_FLAGS(QBluetoothDeviceInfo::ServiceClasses)
    serviceResult = QBluetoothDeviceInfo::CapturingService | serviceFlag1;
    QVERIFY(serviceResult.testFlag(QBluetoothDeviceInfo::AudioService));
    QVERIFY(serviceResult.testFlag(QBluetoothDeviceInfo::CapturingService));
}

void tst_QBluetoothDeviceInfo::tst_manufacturerData()
{
    const int manufacturerAVM = 0x1F;

    QBluetoothDeviceInfo info;
    QVERIFY(info.manufacturerIds().isEmpty());
    QVERIFY(info.manufacturerData(manufacturerAVM).isNull());

    QVERIFY(info.setManufacturerData(manufacturerAVM, QByteArray::fromHex("ABCD")));
    QVERIFY(!info.setManufacturerData(manufacturerAVM, QByteArray::fromHex("ABCD")));
    QCOMPARE(info.manufacturerData(manufacturerAVM), QByteArray::fromHex("ABCD"));
    auto temp = info.manufacturerData();
    QCOMPARE(temp.keys().size(), 1);
    QCOMPARE(temp.values().size(), 1);
    QCOMPARE(temp.values(), QList<QByteArray>() << QByteArray::fromHex("ABCD"));

    QVERIFY(info.setManufacturerData(manufacturerAVM, QByteArray::fromHex("CDEF")));
    QVERIFY(!info.setManufacturerData(manufacturerAVM, QByteArray::fromHex("ABCD")));
    QVERIFY(!info.setManufacturerData(manufacturerAVM, QByteArray::fromHex("CDEF")));

    temp = info.manufacturerData();
    QCOMPARE(temp.keys().size(), 2);
    QCOMPARE(temp.values().size(), 2);
    auto list = temp.values();

    QCOMPARE(QSet<QByteArray> (list.begin(), list.end()),
             QSet<QByteArray>() << QByteArray::fromHex("ABCD") << QByteArray::fromHex("CDEF"));

    // return latest entry
    QCOMPARE(info.manufacturerData(manufacturerAVM), QByteArray::fromHex("CDEF"));
}

QTEST_MAIN(tst_QBluetoothDeviceInfo)

#include "tst_qbluetoothdeviceinfo.moc"
