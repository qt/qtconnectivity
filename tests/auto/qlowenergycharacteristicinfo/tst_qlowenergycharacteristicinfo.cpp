/***************************************************************************
**
** Copyright (C) 2013 BlackBerry Limited all rights reserved
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QUuid>

#include <QDebug>

#include <qbluetoothdeviceinfo.h>
#include <qlowenergycharacteristicinfo.h>
#include <qbluetoothaddress.h>
#include <qbluetoothlocaldevice.h>
#include <qbluetoothuuid.h>

QT_USE_NAMESPACE

Q_DECLARE_METATYPE(QUuid)
Q_DECLARE_METATYPE(QLowEnergyCharacteristicInfo)
Q_DECLARE_METATYPE(QBluetoothUuid::CharacteristicType)

class tst_QLowEnergyCharacteristicInfo : public QObject
{
    Q_OBJECT

public:
    tst_QLowEnergyCharacteristicInfo();
    ~tst_QLowEnergyCharacteristicInfo();

private slots:
    void initTestCase();
    void tst_construction();
    void tst_assignment_data();
    void tst_assignment();
};

tst_QLowEnergyCharacteristicInfo::tst_QLowEnergyCharacteristicInfo()
{
}

tst_QLowEnergyCharacteristicInfo::~tst_QLowEnergyCharacteristicInfo()
{
}

void tst_QLowEnergyCharacteristicInfo::initTestCase()
{
    // start Bluetooth if not started
    QBluetoothLocalDevice *device = new QBluetoothLocalDevice();
    device->powerOn();
    delete device;
}

void tst_QLowEnergyCharacteristicInfo::tst_construction()
{
    const QBluetoothUuid characteristicUuid(QBluetoothUuid::HIDControlPoint);
    const QBluetoothUuid alternateCharacteristicUuid(QBluetoothUuid::TemperatureMeasurement);

    {
        QLowEnergyCharacteristicInfo characteristicInfo;

        QVERIFY(!characteristicInfo.isValid());
        QCOMPARE(characteristicInfo.uuid().toString(), QBluetoothUuid().toString());
        QCOMPARE(characteristicInfo.value(), QByteArray());
        QCOMPARE(characteristicInfo.properties(), QLowEnergyCharacteristicInfo::Unknown);
        QCOMPARE(characteristicInfo.handle(), QString("0x0000"));
        QCOMPARE(characteristicInfo.name(), QString(""));
        QCOMPARE(characteristicInfo.isNotificationCharacteristic(), false);
        QCOMPARE(characteristicInfo.descriptors().count(), 0);
    }

    {
        QLowEnergyCharacteristicInfo characteristicInfo(characteristicUuid);

        QVERIFY(!characteristicInfo.isValid());

        QCOMPARE(characteristicInfo.uuid().toString(), characteristicUuid.toString());
        QCOMPARE(characteristicInfo.value(), QByteArray());
        QCOMPARE(characteristicInfo.properties(), QLowEnergyCharacteristicInfo::Unknown);
        QCOMPARE(characteristicInfo.handle(), QString("0x0000"));
        QCOMPARE(characteristicInfo.name(),
                 QBluetoothUuid::characteristicToString(QBluetoothUuid::HIDControlPoint));
        QCOMPARE(characteristicInfo.isNotificationCharacteristic(), false);
        QCOMPARE(characteristicInfo.descriptors().count(), 0);

        QLowEnergyCharacteristicInfo copyInfo(characteristicInfo);
        QVERIFY(!copyInfo.isValid());
        QCOMPARE(copyInfo.uuid().toString(), characteristicUuid.toString());
        QCOMPARE(copyInfo.value(), QByteArray());
        copyInfo.setValue(QByteArray("test"));
        QCOMPARE(copyInfo.value(), QByteArray("test"));

        //QLowEnergyCharacteristicInfos share their internal data
        //for now enshrine this in the test
        //TODO do we really need this behavior as it is unusual for value types
        QCOMPARE(characteristicInfo.value(), QByteArray("test"));


        copyInfo = QLowEnergyCharacteristicInfo(alternateCharacteristicUuid);
        QCOMPARE(copyInfo.uuid().toString(), alternateCharacteristicUuid.toString());

        QCOMPARE(copyInfo.handle(), QString("0x0000"));
        QCOMPARE(copyInfo.value(), QByteArray());
        QCOMPARE(copyInfo.properties(), QLowEnergyCharacteristicInfo::Unknown);
        QCOMPARE(copyInfo.handle(), QString("0x0000"));
        QCOMPARE(copyInfo.name(),
                 QBluetoothUuid::characteristicToString(QBluetoothUuid::TemperatureMeasurement));
        QCOMPARE(copyInfo.isNotificationCharacteristic(), false);
        QCOMPARE(copyInfo.descriptors().count(), 0);
        QCOMPARE(copyInfo.value(), QByteArray());
        copyInfo.setValue(QByteArray("test"));
        QCOMPARE(copyInfo.value(), QByteArray("test"));
    }
}

void tst_QLowEnergyCharacteristicInfo::tst_assignment_data()
{
    QTest::addColumn<QBluetoothUuid>("characteristicClassUuid");

    QTest::newRow("0x000000 COD") << QBluetoothUuid(QBluetoothUuid::AlertCategoryID);
    QTest::newRow("0x001000 COD") << QBluetoothUuid(QBluetoothUuid::AlertCategoryIDBitMask);
    QTest::newRow("0x002000 COD") << QBluetoothUuid(QBluetoothUuid::AlertLevel);
    QTest::newRow("0x003000 COD") << QBluetoothUuid(QBluetoothUuid::AlertNotificationControlPoint);
    QTest::newRow("0x004000 COD") << QBluetoothUuid(QBluetoothUuid::AlertStatus);
    QTest::newRow("0x005000 COD") << QBluetoothUuid(QBluetoothUuid::Appearance);
    QTest::newRow("0x006000 COD") << QBluetoothUuid(QBluetoothUuid::CSCFeature);
    QTest::newRow("0x007000 COD") << QBluetoothUuid(QBluetoothUuid::CSCMeasurement);
    QTest::newRow("0x008000 COD") << QBluetoothUuid(QBluetoothUuid::CurrentTime);
    QTest::newRow("0x009000 COD") << QBluetoothUuid(QBluetoothUuid::DateTime);
    QTest::newRow("0x010000 COD") << QBluetoothUuid(QBluetoothUuid::DayOfWeek);
    QTest::newRow("0x011000 COD") << QBluetoothUuid(QBluetoothUuid::DeviceName);
    QTest::newRow("0x012000 COD") << QBluetoothUuid(QBluetoothUuid::DSTOffset);
    QTest::newRow("0x013000 COD") << QBluetoothUuid(QBluetoothUuid::ExactTime256);
    QTest::newRow("0x014000 COD") << QBluetoothUuid(QBluetoothUuid::HeartRateControlPoint);
    QTest::newRow("0x015000 COD") << QBluetoothUuid(QBluetoothUuid::IntermediateCuffPressure);
    QTest::newRow("0x016000 COD") << QBluetoothUuid(QBluetoothUuid::Navigation);
    QTest::newRow("0x017000 COD") << QBluetoothUuid(QBluetoothUuid::NewAlert);
    QTest::newRow("0x018000 COD") << QBluetoothUuid(QBluetoothUuid::PeripheralPreferredConnectionParameters);
}

void tst_QLowEnergyCharacteristicInfo::tst_assignment()
{
    QFETCH(QBluetoothUuid, characteristicClassUuid);

    QLowEnergyCharacteristicInfo characteristicInfo(characteristicClassUuid);

    QVERIFY(!characteristicInfo.isValid());

    {
        QLowEnergyCharacteristicInfo copyInfo = characteristicInfo;

        QVERIFY(!copyInfo.isValid());

        QCOMPARE(copyInfo.uuid(), characteristicClassUuid);
        QCOMPARE(copyInfo.value(), QByteArray());
    }

    {
        QLowEnergyCharacteristicInfo copyInfo;

        QVERIFY(!copyInfo.isValid());

        copyInfo = characteristicInfo;

        QVERIFY(!copyInfo.isValid());

        QCOMPARE(copyInfo.uuid(), characteristicClassUuid);
    }

    {
        QLowEnergyCharacteristicInfo copyInfo1;
        QLowEnergyCharacteristicInfo copyInfo2;

        QVERIFY(!copyInfo1.isValid());
        QVERIFY(!copyInfo2.isValid());

        copyInfo1 = copyInfo2 = characteristicInfo;

        QVERIFY(!copyInfo1.isValid());
        QVERIFY(!copyInfo2.isValid());
        //QVERIFY(QLowEnergyCharacteristicInfo() != copyInfo1);

        QCOMPARE(copyInfo1.uuid(), characteristicClassUuid);
        QCOMPARE(copyInfo2.uuid(), characteristicClassUuid);
    }
}

QTEST_MAIN(tst_QLowEnergyCharacteristicInfo)

#include "tst_qlowenergycharacteristicinfo.moc"
