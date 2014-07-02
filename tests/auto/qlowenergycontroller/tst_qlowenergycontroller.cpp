/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
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
#include <QBluetoothLocalDevice>
#include <QBluetoothServiceDiscoveryAgent>
#include <QBluetoothUuid>
#include <QLowEnergyController>
#include <QLowEnergyControllerNew>
#include <QLowEnergyServiceInfo>
#include <QLowEnergyCharacteristic>

#include <QDebug>

QT_USE_NAMESPACE

class tst_QLowEnergyController : public QObject
{
    Q_OBJECT

public:
    tst_QLowEnergyController();
    ~tst_QLowEnergyController();

protected slots:
    void discoveryError(QBluetoothServiceDiscoveryAgent::Error error);
    void serviceDiscovered(const QLowEnergyServiceInfo &info);

private slots:
    void initTestCase();
    void cleanupTestCase();
    void tst_verifyAllServices();
    void tst_connect();
    void tst_connectNew();
    void tst_concurrentDiscovery();
    void tst_defaultBehavior();
    void tst_writeCharacteristic();

private:
    void verifyServiceProperties(const QLowEnergyServiceInfo &info);
    void verifyServiceProperties(const QLowEnergyService *info);

    QBluetoothServiceDiscoveryAgent *agent;
    QBluetoothAddress remoteDevice;
    QList<QLowEnergyServiceInfo> foundServices;
};

Q_DECLARE_METATYPE(QLowEnergyCharacteristic)
Q_DECLARE_METATYPE(QLowEnergyService::ServiceError)


tst_QLowEnergyController::tst_QLowEnergyController()
{
    qRegisterMetaType<QLowEnergyServiceInfo>("QLowEnergyServiceInfo");
    qRegisterMetaType<QLowEnergyController::Error>("QLowEnergyController::Error");
    //qRegisterMetaType<QLowEnergyService::ServiceError>();
    qRegisterMetaType<QLowEnergyCharacteristicInfo>("QLowEnergyCharacteristicInfo");
    qRegisterMetaType<QLowEnergyCharacteristic>();

    QLoggingCategory::setFilterRules(QStringLiteral("qt.bluetooth* = true"));
    const QString remote = qgetenv("BT_TEST_DEVICE");
    if (!remote.isEmpty()) {
        remoteDevice = QBluetoothAddress(remote);
        qWarning() << "Using remote device " << remote << " for testing. Ensure that the device is discoverable for pairing requests";
    } else {
        qWarning() << "Not using any remote device for testing. Set BT_TEST_DEVICE env to run manual tests involving a remote device";
    }

}

tst_QLowEnergyController::~tst_QLowEnergyController()
{

}

void tst_QLowEnergyController::discoveryError(QBluetoothServiceDiscoveryAgent::Error /*error*/)
{
}

void tst_QLowEnergyController::serviceDiscovered(const QLowEnergyServiceInfo &info)
{
    foundServices.append(info);
}

void tst_QLowEnergyController::initTestCase()
{
    if (remoteDevice.isNull()
        || QBluetoothLocalDevice::allDevices().isEmpty()) {
        qWarning("No remote device found");
        return;
    }
    agent = new QBluetoothServiceDiscoveryAgent(this);
    connect(agent, SIGNAL(serviceDiscovered(QLowEnergyServiceInfo)),
            SLOT(serviceDiscovered(QLowEnergyServiceInfo)));
    connect(agent, SIGNAL(error(QBluetoothServiceDiscoveryAgent::Error)),
            SLOT(discoveryError(QBluetoothServiceDiscoveryAgent::Error)));

    agent->setRemoteAddress(remoteDevice);

    QSignalSpy spy(agent, SIGNAL(finished()));
    // there should be no changes yet
    QVERIFY(spy.isValid());
    QVERIFY(spy.isEmpty());


    agent->start(QBluetoothServiceDiscoveryAgent::FullDiscovery);

    QTRY_VERIFY(spy.count() > 0);
    QCOMPARE(12, foundServices.count());
}

void tst_QLowEnergyController::cleanupTestCase()
{

}

void tst_QLowEnergyController::tst_verifyAllServices()
{
    QList<QBluetoothUuid> uuids;
    uuids << QBluetoothUuid(QString("00001800-0000-1000-8000-00805f9b34fb"));
    uuids << QBluetoothUuid(QString("00001801-0000-1000-8000-00805f9b34fb"));
    uuids << QBluetoothUuid(QString("0000180a-0000-1000-8000-00805f9b34fb"));
    uuids << QBluetoothUuid(QString("0000ffe0-0000-1000-8000-00805f9b34fb"));
    uuids << QBluetoothUuid(QString("f000aa00-0451-4000-b000-000000000000"));
    uuids << QBluetoothUuid(QString("f000aa10-0451-4000-b000-000000000000"));
    uuids << QBluetoothUuid(QString("f000aa20-0451-4000-b000-000000000000"));
    uuids << QBluetoothUuid(QString("f000aa30-0451-4000-b000-000000000000"));
    uuids << QBluetoothUuid(QString("f000aa40-0451-4000-b000-000000000000"));
    uuids << QBluetoothUuid(QString("f000aa50-0451-4000-b000-000000000000"));
    uuids << QBluetoothUuid(QString("f000aa60-0451-4000-b000-000000000000"));
    uuids << QBluetoothUuid(QString("f000ffc0-0451-4000-b000-000000000000"));

    foreach (const QLowEnergyServiceInfo &info, foundServices) {
        QVERIFY(uuids.contains(info.serviceUuid()));
        //QVERIFY(!info.isConnected());
        QVERIFY(info.isValid());
        QCOMPARE(QLowEnergyServiceInfo::PrimaryService, info.serviceType());
        QBluetoothUuid u = info.serviceUuid();
        bool ok = false;
        quint16 clsId = u.toUInt16(&ok);
        if (ok && (clsId & 0x1800) == 0x1800 && clsId < 0x1900) {
            QVERIFY(!info.serviceName().isEmpty());
        } else {
            const QString unknown = info.serviceName();
            QVERIFY2(unknown.isEmpty() || unknown == QStringLiteral("Unknown Service"),
                     info.serviceUuid().toString().toLatin1());
        }

        QVERIFY(info.characteristics().isEmpty());
        QCOMPARE(remoteDevice, info.device().address());
    }
}

void tst_QLowEnergyController::verifyServiceProperties(const QLowEnergyServiceInfo &info)
{
    if (info.serviceUuid() ==
            QBluetoothUuid(QString("00001800-0000-1000-8000-00805f9b34fb"))) {
        qDebug() << "Verifying GAP Service";
        QList<QLowEnergyCharacteristicInfo> chars = info.characteristics();
        QCOMPARE(chars.count(), 5);

        // Device Name
        QString temp("00002a00-0000-1000-8000-00805f9b34fb");
        QCOMPARE(chars[0].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[0].handle(), 0x3u);
        QCOMPARE(chars[0].properties(), QLowEnergyCharacteristicInfo::Read);
        QCOMPARE(chars[0].value(), QByteArray("544920424c452053656e736f7220546167"));
        QVERIFY(!chars[0].isNotificationCharacteristic());
        QVERIFY(chars[0].isValid());
        QCOMPARE(chars[0].descriptors().count(), 0);

        // Appearance
        temp = QString("00002a01-0000-1000-8000-00805f9b34fb");
        QCOMPARE(chars[1].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[1].handle(), 0x5u);
        QCOMPARE(chars[1].properties(), QLowEnergyCharacteristicInfo::Read);
        QCOMPARE(chars[1].value(), QByteArray("30303030"));
        QVERIFY(!chars[1].isNotificationCharacteristic());
        QVERIFY(chars[1].isValid());
        QCOMPARE(chars[1].descriptors().count(), 0);

        // Peripheral Privacy Flag
        temp = QString("00002a02-0000-1000-8000-00805f9b34fb");
        QCOMPARE(chars[2].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[2].handle(), 0x7u);
        QCOMPARE(chars[2].properties(),
                 (QLowEnergyCharacteristicInfo::Read|QLowEnergyCharacteristicInfo::Write));
        QCOMPARE(chars[2].value(), QByteArray("00"));
        QVERIFY(!chars[2].isNotificationCharacteristic());
        QVERIFY(chars[2].isValid());
        QCOMPARE(chars[2].descriptors().count(), 0);

        // Reconnection Address
        temp = QString("00002a03-0000-1000-8000-00805f9b34fb");
        QCOMPARE(chars[3].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[3].handle(), 0x9u);
        QCOMPARE(chars[3].properties(),
                 (QLowEnergyCharacteristicInfo::Read|QLowEnergyCharacteristicInfo::Write));
        QCOMPARE(chars[3].value(), QByteArray("000000000000"));
        QVERIFY(!chars[3].isNotificationCharacteristic());
        QVERIFY(chars[3].isValid());
        QCOMPARE(chars[3].descriptors().count(), 0);

        // Peripheral Preferred Connection Parameters
        temp = QString("00002a04-0000-1000-8000-00805f9b34fb");
        QCOMPARE(chars[4].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[4].handle(), 0xbu);
        QCOMPARE(chars[4].properties(), QLowEnergyCharacteristicInfo::Read);
        QCOMPARE(chars[4].value(), QByteArray("5000a0000000e803"));
        QVERIFY(!chars[4].isNotificationCharacteristic());
        QVERIFY(chars[4].isValid());
        QCOMPARE(chars[4].descriptors().count(), 0);
    } else if (info.serviceUuid() ==
                QBluetoothUuid(QString("00001801-0000-1000-8000-00805f9b34fb"))) {
        qDebug() << "Verifying GATT Service";
        QList<QLowEnergyCharacteristicInfo> chars = info.characteristics();
        QCOMPARE(chars.count(), 1);

        // Service Changed
        QString temp("00002a05-0000-1000-8000-00805f9b34fb");
        //this should really be readable according to GATT Service spec
        QCOMPARE(chars[0].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[0].handle(), 0xeu);
        QCOMPARE(chars[0].properties(), QLowEnergyCharacteristicInfo::Indicate);
        QCOMPARE(chars[0].value(), QByteArray(""));
        QVERIFY(!chars[0].isNotificationCharacteristic());  //TODO should this really be false
        QVERIFY(chars[0].isValid());
        QCOMPARE(chars[0].descriptors().count(), 0);
    } else if (info.serviceUuid() ==
                QBluetoothUuid(QString("0000180a-0000-1000-8000-00805f9b34fb"))) {
        qDebug() << "Verifying Device Information";
        QList<QLowEnergyCharacteristicInfo> chars = info.characteristics();
        QCOMPARE(chars.count(), 9);

        // System ID
        QString temp("00002a23-0000-1000-8000-00805f9b34fb");
        //this should really be readable according to GATT Service spec
        QCOMPARE(chars[0].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[0].handle(), 0x12u);
        QCOMPARE(chars[0].properties(), QLowEnergyCharacteristicInfo::Read);
        QCOMPARE(chars[0].value(), QByteArray("6e41ab0000296abc"));
        QVERIFY(!chars[0].isNotificationCharacteristic());
        QVERIFY(chars[0].isValid());
        QCOMPARE(chars[0].descriptors().count(), 0);

        // Model Number
        temp = QString("00002a24-0000-1000-8000-00805f9b34fb");
        QCOMPARE(chars[1].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[1].handle(), 0x14u);
        QCOMPARE(chars[1].properties(),
                 (QLowEnergyCharacteristicInfo::Read));
        QCOMPARE(chars[1].value(), QByteArray("4e2e412e00"));
        QVERIFY(!chars[1].isNotificationCharacteristic());
        QVERIFY(chars[1].isValid());
        QCOMPARE(chars[1].descriptors().count(), 0);

        // Serial Number
        temp = QString("00002a25-0000-1000-8000-00805f9b34fb");
        QCOMPARE(chars[2].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[2].handle(), 0x16u);
        QCOMPARE(chars[2].properties(),
                 (QLowEnergyCharacteristicInfo::Read));
        QCOMPARE(chars[2].value(), QByteArray("4e2e412e00"));
        QVERIFY(!chars[2].isNotificationCharacteristic());
        QVERIFY(chars[2].isValid());
        QCOMPARE(chars[2].descriptors().count(), 0);

        // Firmware Revision
        temp = QString("00002a26-0000-1000-8000-00805f9b34fb");
        QCOMPARE(chars[3].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[3].handle(), 0x18u);
        QCOMPARE(chars[3].properties(),
                 (QLowEnergyCharacteristicInfo::Read));
        QCOMPARE(chars[3].value(), QByteArray("312e3031202846656220203720323031332900"));
        QVERIFY(!chars[3].isNotificationCharacteristic());
        QVERIFY(chars[3].isValid());
        QCOMPARE(chars[3].descriptors().count(), 0);

        // Hardware Revision
        temp = QString("00002a27-0000-1000-8000-00805f9b34fb");
        QCOMPARE(chars[4].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[4].handle(), 0x1au);
        QCOMPARE(chars[4].properties(),
                 (QLowEnergyCharacteristicInfo::Read));
        QCOMPARE(chars[4].value(), QByteArray("4e2e412e00"));
        QVERIFY(!chars[4].isNotificationCharacteristic());
        QVERIFY(chars[4].isValid());
        QCOMPARE(chars[4].descriptors().count(), 0);

        // Software Revision
        temp = QString("00002a28-0000-1000-8000-00805f9b34fb");
        QCOMPARE(chars[5].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[5].handle(), 0x1cu);
        QCOMPARE(chars[5].properties(),
                 (QLowEnergyCharacteristicInfo::Read));
        QCOMPARE(chars[5].value(), QByteArray("4e2e412e00"));
        QVERIFY(!chars[5].isNotificationCharacteristic());
        QVERIFY(chars[5].isValid());
        QCOMPARE(chars[5].descriptors().count(), 0);

        // Manufacturer Name
        temp = QString("00002a29-0000-1000-8000-00805f9b34fb");
        QCOMPARE(chars[6].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[6].handle(), 0x1eu);
        QCOMPARE(chars[6].properties(),
                 (QLowEnergyCharacteristicInfo::Read));
        QCOMPARE(chars[6].value(), QByteArray("546578617320496e737472756d656e747300"));
        QVERIFY(!chars[6].isNotificationCharacteristic());
        QVERIFY(chars[6].isValid());
        QCOMPARE(chars[6].descriptors().count(), 0);

        // IEEE
        temp = QString("00002a2a-0000-1000-8000-00805f9b34fb");
        QCOMPARE(chars[7].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[7].handle(), 0x20u);
        QCOMPARE(chars[7].properties(),
                 (QLowEnergyCharacteristicInfo::Read));
        QCOMPARE(chars[7].value(), QByteArray("fe006578706572696d656e74616c"));
        QVERIFY(!chars[7].isNotificationCharacteristic());
        QVERIFY(chars[7].isValid());
        QCOMPARE(chars[7].descriptors().count(), 0);

        // PnP ID
        temp = QString("00002a50-0000-1000-8000-00805f9b34fb");
        QCOMPARE(chars[8].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[8].handle(), 0x22u);
        QCOMPARE(chars[8].properties(),
                 (QLowEnergyCharacteristicInfo::Read));
        QCOMPARE(chars[8].value(), QByteArray("010d0000001001"));
        QVERIFY(!chars[8].isNotificationCharacteristic());
        QVERIFY(chars[8].isValid());
        QCOMPARE(chars[8].descriptors().count(), 0);
    } else if (info.serviceUuid() ==
               QBluetoothUuid(QString("f000aa00-0451-4000-b000-000000000000"))) {
        qDebug() << "Verifying Temperature";
        QList<QLowEnergyCharacteristicInfo> chars = info.characteristics();
        QCOMPARE(chars.count(), 2);

        // Temp Data
        QString temp("f000aa01-0451-4000-b000-000000000000");
        QCOMPARE(chars[0].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[0].handle(), 0x25u);
        QCOMPARE(chars[0].properties(),
                (QLowEnergyCharacteristicInfo::Read|QLowEnergyCharacteristicInfo::Notify));
        //QCOMPARE(chars[0].value(), QByteArray("30303030"));
        QVERIFY(chars[0].isNotificationCharacteristic());
        QVERIFY(chars[0].isValid());
        QCOMPARE(chars[0].descriptors().count(), 1);

        QCOMPARE(chars[0].descriptors().at(0).handle(), 0x26u);
        QCOMPARE(chars[0].descriptors().at(0).uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[0].descriptors().at(0).value(), QByteArray("30303030"));
        temp = QString("00002902-0000-1000-8000-00805f9b34fb");
        QCOMPARE(QBluetoothUuid(chars[0].descriptors().at(0).type()), QBluetoothUuid(temp));

        // Temp Config
        temp = QString("f000aa02-0451-4000-b000-000000000000");
        QCOMPARE(chars[1].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[1].handle(), 0x29u);
        QCOMPARE(chars[1].properties(),
                 (QLowEnergyCharacteristicInfo::Read|QLowEnergyCharacteristicInfo::Write));
        QCOMPARE(chars[1].value(), QByteArray("00"));
        QVERIFY(!chars[1].isNotificationCharacteristic());
        QVERIFY(chars[1].isValid());
        QCOMPARE(chars[1].descriptors().count(), 0);
    } else if (info.serviceUuid() ==
               QBluetoothUuid(QString("0000ffe0-0000-1000-8000-00805f9b34fb"))) {
        qDebug() << "Verifying Simple Keys";
        QList<QLowEnergyCharacteristicInfo> chars = info.characteristics();
        QCOMPARE(chars.count(), 1);

        // Temp Data
        QString temp("0000ffe1-0000-1000-8000-00805f9b34fb");
        QCOMPARE(chars[0].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[0].handle(), 0x5fu);
        QCOMPARE(chars[0].properties(),
                (QLowEnergyCharacteristicInfo::Notify));
        QCOMPARE(chars[0].value(), QByteArray(""));
        QEXPECT_FAIL("", "0x005f has a notification", Continue);
        QVERIFY(chars[0].isNotificationCharacteristic());
        QVERIFY(chars[0].isValid());
        QCOMPARE(chars[0].descriptors().count(), 0); //TODO should be 1?
    } else if (info.serviceUuid() ==
               QBluetoothUuid(QString("f000aa10-0451-4000-b000-000000000000"))) {
        qDebug() << "Verifying Accelerometer";
        QList<QLowEnergyCharacteristicInfo> chars = info.characteristics();
        QCOMPARE(chars.count(), 3);

        // Accel Data
        QString temp("f000aa11-0451-4000-b000-000000000000");
        QCOMPARE(chars[0].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[0].handle(), 0x2du);
        QCOMPARE(chars[0].properties(),
                (QLowEnergyCharacteristicInfo::Read|QLowEnergyCharacteristicInfo::Notify));
        QCOMPARE(chars[0].value(), QByteArray("000000"));
        QVERIFY(chars[0].isNotificationCharacteristic());
        QVERIFY(chars[0].isValid());
        QCOMPARE(chars[0].descriptors().count(), 1);

        QCOMPARE(chars[0].descriptors().at(0).handle(), 0x2eu);
        QCOMPARE(chars[0].descriptors().at(0).uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[0].descriptors().at(0).value(), QByteArray("30303030"));
        temp = QString("00002902-0000-1000-8000-00805f9b34fb");
        QCOMPARE(QBluetoothUuid(chars[0].descriptors().at(0).type()), QBluetoothUuid(temp));

        // Accel Config
        temp = QString("f000aa12-0451-4000-b000-000000000000");
        QCOMPARE(chars[1].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[1].handle(), 0x31u);
        QCOMPARE(chars[1].properties(),
                 (QLowEnergyCharacteristicInfo::Read|QLowEnergyCharacteristicInfo::Write));
        QCOMPARE(chars[1].value(), QByteArray("00"));
        QVERIFY(!chars[1].isNotificationCharacteristic());
        QVERIFY(chars[1].isValid());
        QCOMPARE(chars[1].descriptors().count(), 0);

        // Accel Period
        temp = QString("f000aa13-0451-4000-b000-000000000000");
        QCOMPARE(chars[2].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[2].handle(), 0x34u);
        QCOMPARE(chars[2].properties(),
                 (QLowEnergyCharacteristicInfo::Read|QLowEnergyCharacteristicInfo::Write));
        QCOMPARE(chars[2].value(), QByteArray("64"));   // don't change it or set it to 0x64
        QVERIFY(!chars[2].isNotificationCharacteristic());
        QVERIFY(chars[2].isValid());
        QCOMPARE(chars[2].descriptors().count(), 0);
    } else if (info.serviceUuid() ==
               QBluetoothUuid(QString("f000aa20-0451-4000-b000-000000000000"))) {
        qDebug() << "Verifying Humidity";
        QList<QLowEnergyCharacteristicInfo> chars = info.characteristics();
        QCOMPARE(chars.count(), 2);

        // Humidity Data
        QString temp("f000aa21-0451-4000-b000-000000000000");
        QCOMPARE(chars[0].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[0].handle(), 0x38u);
        QCOMPARE(chars[0].properties(),
                (QLowEnergyCharacteristicInfo::Read|QLowEnergyCharacteristicInfo::Notify));
        QCOMPARE(chars[0].value(), QByteArray("00000000"));
        QVERIFY(chars[0].isNotificationCharacteristic());
        QVERIFY(chars[0].isValid());
        QCOMPARE(chars[0].descriptors().count(), 1);

        QCOMPARE(chars[0].descriptors().at(0).handle(), 0x39u);
        QCOMPARE(chars[0].descriptors().at(0).uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[0].descriptors().at(0).value(), QByteArray("30303030"));
        temp = QString("00002902-0000-1000-8000-00805f9b34fb");
        QCOMPARE(QBluetoothUuid(chars[0].descriptors().at(0).type()), QBluetoothUuid(temp));

        // Humidity Config
        temp = QString("f000aa22-0451-4000-b000-000000000000");
        QCOMPARE(chars[1].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[1].handle(), 0x3cu);
        QCOMPARE(chars[1].properties(),
                 (QLowEnergyCharacteristicInfo::Read|QLowEnergyCharacteristicInfo::Write));
        QCOMPARE(chars[1].value(), QByteArray("00"));
        QVERIFY(!chars[1].isNotificationCharacteristic());
        QVERIFY(chars[1].isValid());
        QCOMPARE(chars[1].descriptors().count(), 0);
    } else if (info.serviceUuid() ==
               QBluetoothUuid(QString("f000aa30-0451-4000-b000-000000000000"))) {
        qDebug() << "Verifying Magnetometer";
        QList<QLowEnergyCharacteristicInfo> chars = info.characteristics();
        QCOMPARE(chars.count(), 3);

        // Magnetometer Data
        QString temp("f000aa31-0451-4000-b000-000000000000");
        QCOMPARE(chars[0].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[0].handle(), 0x40u);
        QCOMPARE(chars[0].properties(),
                (QLowEnergyCharacteristicInfo::Read|QLowEnergyCharacteristicInfo::Notify));
        QCOMPARE(chars[0].value(), QByteArray("000000000000"));
        QVERIFY(chars[0].isNotificationCharacteristic());
        QVERIFY(chars[0].isValid());
        QCOMPARE(chars[0].descriptors().count(), 1);

        QCOMPARE(chars[0].descriptors().at(0).handle(), 0x41u);
        QCOMPARE(chars[0].descriptors().at(0).uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[0].descriptors().at(0).value(), QByteArray("30303030"));
        temp = QString("00002902-0000-1000-8000-00805f9b34fb");
        QCOMPARE(QBluetoothUuid(chars[0].descriptors().at(0).type()), QBluetoothUuid(temp));

        // Magnetometer Config
        temp = QString("f000aa32-0451-4000-b000-000000000000");
        QCOMPARE(chars[1].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[1].handle(), 0x44u);
        QCOMPARE(chars[1].properties(),
                 (QLowEnergyCharacteristicInfo::Read|QLowEnergyCharacteristicInfo::Write));
        QCOMPARE(chars[1].value(), QByteArray("00"));
        QVERIFY(!chars[1].isNotificationCharacteristic());
        QVERIFY(chars[1].isValid());
        QCOMPARE(chars[1].descriptors().count(), 0);

        // Magnetometer Period
        temp = QString("f000aa33-0451-4000-b000-000000000000");
        QCOMPARE(chars[2].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[2].handle(), 0x47u);
        QCOMPARE(chars[2].properties(),
                 (QLowEnergyCharacteristicInfo::Read|QLowEnergyCharacteristicInfo::Write));
        QCOMPARE(chars[2].value(), QByteArray("c8"));   // don't change it or set it to 0xc8
        QVERIFY(!chars[2].isNotificationCharacteristic());
        QVERIFY(chars[2].isValid());
        QCOMPARE(chars[2].descriptors().count(), 0);
    } else if (info.serviceUuid() ==
               QBluetoothUuid(QString("f000aa40-0451-4000-b000-000000000000"))) {
        qDebug() << "Verifying Pressure";
        QList<QLowEnergyCharacteristicInfo> chars = info.characteristics();
        QCOMPARE(chars.count(), 3);

        // Pressure Data
        QString temp("f000aa41-0451-4000-b000-000000000000");
        QCOMPARE(chars[0].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[0].handle(), 0x4bu);
        QCOMPARE(chars[0].properties(),
                (QLowEnergyCharacteristicInfo::Read|QLowEnergyCharacteristicInfo::Notify));
        QCOMPARE(chars[0].value(), QByteArray("00000000"));
        QEXPECT_FAIL("", "there is a notification 0x004c", Continue);
        QCOMPARE(chars[0].isNotificationCharacteristic(), true);
        QVERIFY(chars[0].isValid());
        QEXPECT_FAIL("", "0x4b has at least a notification", Continue);
        QCOMPARE(chars[0].descriptors().count(), 1);

        if (!chars[0].descriptors().isEmpty()) {
            temp = QString("00002902-0000-1000-8000-00805f9b34fb");
            QCOMPARE(chars[0].descriptors().at(0).handle(), 0x41u);
            QCOMPARE(chars[0].descriptors().at(0).uuid(), QBluetoothUuid(temp));
            QCOMPARE(chars[0].descriptors().at(0).value(), QByteArray("30303030"));
        }

        // Pressure Config
        temp = QString("f000aa42-0451-4000-b000-000000000000");
        QCOMPARE(chars[1].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[1].handle(), 0x4fu);
        QCOMPARE(chars[1].properties(),
                 (QLowEnergyCharacteristicInfo::Read|QLowEnergyCharacteristicInfo::Write));
        QCOMPARE(chars[1].value(), QByteArray("00"));
        QVERIFY(!chars[1].isNotificationCharacteristic());
        QVERIFY(chars[1].isValid());
        QCOMPARE(chars[1].descriptors().count(), 0);

        // Pressure Calibration
        temp = QString("f000aa43-0451-4000-b000-000000000000");
        QCOMPARE(chars[2].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[2].handle(), 0x52u);
        QCOMPARE(chars[2].properties(),
                 (QLowEnergyCharacteristicInfo::Read));
        QCOMPARE(chars[2].value(), QByteArray("00000000000000000000000000000000"));   // don't change it
        QVERIFY(!chars[2].isNotificationCharacteristic());
        QVERIFY(chars[2].isValid());
        QCOMPARE(chars[2].descriptors().count(), 0);
    } else if (info.serviceUuid() ==
               QBluetoothUuid(QString("f000aa50-0451-4000-b000-000000000000"))) {
        qDebug() << "Verifying Gyroscope";
        QList<QLowEnergyCharacteristicInfo> chars = info.characteristics();
        QCOMPARE(chars.count(), 2);

        // Gyroscope Data
        QString temp("f000aa51-0451-4000-b000-000000000000");
        QCOMPARE(chars[0].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[0].handle(), 0x57u);
        QCOMPARE(chars[0].properties(),
                (QLowEnergyCharacteristicInfo::Read|QLowEnergyCharacteristicInfo::Notify));
        QCOMPARE(chars[0].value(), QByteArray("000000000000"));
        QEXPECT_FAIL("", "there is a notification under 0x0057", Continue);
        QVERIFY(chars[0].isNotificationCharacteristic());
        QVERIFY(chars[0].isValid());
        QEXPECT_FAIL("", "there is a notification under 0x0057", Continue);
        QCOMPARE(chars[0].descriptors().count(), 1);

        if (!chars[0].descriptors().isEmpty()) {
            temp = QString("00002902-0000-1000-8000-00805f9b34fb");
            QCOMPARE(chars[0].descriptors().at(0).handle(), 0x39u);
            QCOMPARE(chars[0].descriptors().at(0).uuid(), QBluetoothUuid(temp));
            QCOMPARE(chars[0].descriptors().at(0).value(), QByteArray("30303030"));
        }

        // Gyroscope Config
        temp = QString("f000aa52-0451-4000-b000-000000000000");
        QCOMPARE(chars[1].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[1].handle(), 0x5bu);
        QCOMPARE(chars[1].properties(),
                 (QLowEnergyCharacteristicInfo::Read|QLowEnergyCharacteristicInfo::Write));
        QCOMPARE(chars[1].value(), QByteArray("00"));
        QVERIFY(!chars[1].isNotificationCharacteristic());
        QVERIFY(chars[1].isValid());
        QCOMPARE(chars[1].descriptors().count(), 0);
    } else if (info.serviceUuid() ==
               QBluetoothUuid(QString("f000aa60-0451-4000-b000-000000000000"))) {
        qDebug() << "Verifying Test Service";
        QList<QLowEnergyCharacteristicInfo> chars = info.characteristics();
        QCOMPARE(chars.count(), 2);

        // Test Data
        QString temp("f000aa61-0451-4000-b000-000000000000");
        QCOMPARE(chars[0].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[0].handle(), 0x64u);
        QCOMPARE(chars[0].properties(),
                (QLowEnergyCharacteristicInfo::Read));
        QCOMPARE(chars[0].value(), QByteArray("3f00"));
        QVERIFY(!chars[0].isNotificationCharacteristic());
        QVERIFY(chars[0].isValid());
        QCOMPARE(chars[0].descriptors().count(), 0);

        // Test Config
        temp = QString("f000aa62-0451-4000-b000-000000000000");
        QCOMPARE(chars[1].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[1].handle(), 0x67u);
        QCOMPARE(chars[1].properties(),
                 (QLowEnergyCharacteristicInfo::Read|QLowEnergyCharacteristicInfo::Write));
        QCOMPARE(chars[1].value(), QByteArray("00"));
        QVERIFY(!chars[1].isNotificationCharacteristic());
        QVERIFY(chars[1].isValid());
        QCOMPARE(chars[1].descriptors().count(), 0);
    } else if (info.serviceUuid() ==
               QBluetoothUuid(QString("f000ffc0-0451-4000-b000-000000000000"))) {
        qDebug() << "Verifying Unknown Service";
        QList<QLowEnergyCharacteristicInfo> chars = info.characteristics();
        QCOMPARE(chars.count(), 2);

        // first characteristic
        QString temp("f000ffc1-0451-4000-b000-000000000000");
        QCOMPARE(chars[0].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[0].handle(), 0x6bu);
        QCOMPARE(chars[0].properties(),
                (QLowEnergyCharacteristicInfo::Notify|QLowEnergyCharacteristicInfo::Write|QLowEnergyCharacteristicInfo::WriteNoResponse));
        QCOMPARE(chars[0].value(), QByteArray(""));
        QEXPECT_FAIL("", "there is a notification under 0x006b", Continue);
        QVERIFY(chars[0].isNotificationCharacteristic());
        QVERIFY(chars[0].isValid());
        QEXPECT_FAIL("", "there is a notification under 0x006b", Continue);
        QCOMPARE(chars[0].descriptors().count(), 1);

        if (!chars[0].descriptors().isEmpty()) {
            temp = QString("00002902-0000-1000-8000-00805f9b34fb");
            QCOMPARE(chars[0].descriptors().at(0).handle(), 0x39u);
            QCOMPARE(chars[0].descriptors().at(0).uuid(), QBluetoothUuid(temp));
            QCOMPARE(chars[0].descriptors().at(0).value(), QByteArray("30303030"));
        }

        // second characteristic
        temp = QString("f000ffc2-0451-4000-b000-000000000000");
        QCOMPARE(chars[1].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[1].handle(), 0x6fu);
        QCOMPARE(chars[1].properties(),
                 (QLowEnergyCharacteristicInfo::Notify|QLowEnergyCharacteristicInfo::Write|QLowEnergyCharacteristicInfo::WriteNoResponse));
        QCOMPARE(chars[1].value(), QByteArray(""));
        QEXPECT_FAIL("", "there is a notification under 0x006f", Continue);
        QVERIFY(chars[1].isNotificationCharacteristic());
        QVERIFY(chars[1].isValid());
        QEXPECT_FAIL("", "there is a notification under 0x006f", Continue);
        QCOMPARE(chars[1].descriptors().count(), 1);
    } else {
        QFAIL(QString("Service not found" + info.serviceUuid().toString()).toUtf8().constData());
    }
}

void tst_QLowEnergyController::tst_connect()
{
    QSKIP("Not testing old API at the moment.");
    QLowEnergyController control;
    for (int i = 0; i < foundServices.count(); i++) {

        QLowEnergyServiceInfo info = foundServices.at(i);

        QVERIFY(!info.isConnected());
        QSignalSpy connectedSpy(&control, SIGNAL(connected(QLowEnergyServiceInfo)));
        QVERIFY(connectedSpy.isEmpty());
        QSignalSpy disconnectedSpy(&control, SIGNAL(disconnected(QLowEnergyServiceInfo)));
        QVERIFY(disconnectedSpy.isEmpty());
        QSignalSpy errorService(&control,
                       SIGNAL(error(QLowEnergyServiceInfo,QLowEnergyController::Error)));
        QVERIFY(errorService.isEmpty());
        QSignalSpy errorChar(&control,
                       SIGNAL(error(QLowEnergyCharacteristicInfo,QLowEnergyController::Error)));
        QVERIFY(errorChar.isEmpty());

        control.connectToService(info);

        QTRY_VERIFY_WITH_TIMEOUT(info.isConnected(), 100000);
        QVERIFY(connectedSpy.count() == 1);

        verifyServiceProperties(info);


        control.disconnectFromService(info);
        QTRY_VERIFY(!disconnectedSpy.isEmpty());
    }

//    QLowEnergyServiceInfo info1 = foundServices[0];
//    QLowEnergyServiceInfo info2 = foundServices[1];

//    QVERIFY(!info1.isConnected());
//    QVERIFY(!info2.isConnected());

//    control.connectToService(info1);
//    QTRY_VERIFY_WITH_TIMEOUT(info1.isConnected(), 10000);
//    control.connectToService(info2);
//    QTRY_VERIFY_WITH_TIMEOUT(info2.isConnected(), 10000);

//    control.disconnectFromService(info1);
//    QTRY_VERIFY_WITH_TIMEOUT(!info1.isConnected(), 10000);
//    QVERIFY(info2.isConnected());
//    control.disconnectFromService(info2);
    //    QTRY_VERIFY_WITH_TIMEOUT(!info2.isConnected(), 10000);
}

void tst_QLowEnergyController::tst_connectNew()
{
    QList<QBluetoothHostInfo> localAdapters = QBluetoothLocalDevice::allDevices();
    if (localAdapters.isEmpty() || remoteDevice.isNull())
        QSKIP("No local Bluetooth or remote BTLE device found. Skipping test.");

    const QBluetoothAddress localAdapter = localAdapters.at(0).address();
    QLowEnergyControllerNew control(remoteDevice);
    QSignalSpy connectedSpy(&control, SIGNAL(connected()));
    QSignalSpy disconnectedSpy(&control, SIGNAL(disconnected()));

    QCOMPARE(control.localAddress(), localAdapter);
    QVERIFY(!control.localAddress().isNull());
    QCOMPARE(control.remoteAddress(), remoteDevice);
    QCOMPARE(control.state(), QLowEnergyControllerNew::UnconnectedState);
    QCOMPARE(control.error(), QLowEnergyControllerNew::NoError);
    QVERIFY(control.errorString().isEmpty());
    QCOMPARE(disconnectedSpy.count(), 0);
    QCOMPARE(connectedSpy.count(), 0);
    QVERIFY(control.services().isEmpty());

    bool wasError = false;
    control.connectToDevice();
    QTRY_IMPL(control.state() != QLowEnergyControllerNew::ConnectingState,
              10000);

    QCOMPARE(disconnectedSpy.count(), 0);
    if (control.error() != QLowEnergyControllerNew::NoError) {
        //error during connect
        QCOMPARE(connectedSpy.count(), 0);
        QCOMPARE(control.state(), QLowEnergyControllerNew::UnconnectedState);
        wasError = true;
    } else if (control.state() == QLowEnergyControllerNew::ConnectingState) {
        //timeout
        QCOMPARE(connectedSpy.count(), 0);
        QVERIFY(control.errorString().isEmpty());
        QCOMPARE(control.error(), QLowEnergyControllerNew::NoError);
        QVERIFY(control.services().isEmpty());
        QSKIP("Connection to LE device cannot be established. Skipping test.");
        return;
    } else {
        QCOMPARE(control.state(), QLowEnergyControllerNew::ConnectedState);
        QCOMPARE(connectedSpy.count(), 1);
        QCOMPARE(control.error(), QLowEnergyControllerNew::NoError);
        QVERIFY(control.errorString().isEmpty());
    }

    QVERIFY(control.services().isEmpty());

    QList<QLowEnergyService *> savedReferences;

    if (!wasError) {
        QSignalSpy discoveryFinishedSpy(&control, SIGNAL(discoveryFinished()));
        QSignalSpy serviceFoundSpy(&control, SIGNAL(serviceDiscovered(QBluetoothUuid)));
        control.discoverServices();
        QTRY_VERIFY_WITH_TIMEOUT(discoveryFinishedSpy.count() == 1, 10000);

        QCOMPARE(serviceFoundSpy.count(), foundServices.count());
        QVERIFY(!serviceFoundSpy.isEmpty());
        QList<QBluetoothUuid> listing;
        for (int i = 0; i < serviceFoundSpy.count(); i++) {
            const QVariant v = serviceFoundSpy[i].at(0);
            listing.append(v.value<QBluetoothUuid>());
        }

        foreach (const QLowEnergyServiceInfo &info, foundServices) {
            QVERIFY2(listing.contains(info.serviceUuid()),
                     info.serviceUuid().toString().toLatin1());

            QLowEnergyService *service = control.createServiceObject(info.serviceUuid());
            QVERIFY2(service, info.serviceUuid().toString().toLatin1());
            savedReferences.append(service);
            QCOMPARE(service->type(), QLowEnergyService::PrimaryService);
            QCOMPARE(service->state(), QLowEnergyService::DiscoveryRequired);
        }

        // unrelated uuids don't return valid service object
        // invalid service uuid
        QVERIFY(!control.createServiceObject(QBluetoothUuid()));
        // some random uuid
        QVERIFY(!control.createServiceObject(QBluetoothUuid(QBluetoothUuid::DeviceName)));

        // initiate characteristic discovery
        foreach (QLowEnergyService *service, savedReferences) {
            qDebug() << "Discoverying" << service->serviceUuid();
            QSignalSpy stateSpy(service,
                                SIGNAL(stateChanged(QLowEnergyService::ServiceState)));
            QSignalSpy errorSpy(service, SIGNAL(error(QLowEnergyService::ServiceError)));
            service->discoverDetails();

            QTRY_VERIFY_WITH_TIMEOUT(
                        service->state() == QLowEnergyService::ServiceDiscovered, 10000);

            QCOMPARE(errorSpy.count(), 0); //no error
            QCOMPARE(stateSpy.count(), 2); //

            verifyServiceProperties(service);
        }

        // ensure that related service objects share same state
        foreach (QLowEnergyService* originalService, savedReferences) {
            QLowEnergyService *newService = control.createServiceObject(
                        originalService->serviceUuid());
            QVERIFY(newService);
            QCOMPARE(newService->state(), QLowEnergyService::ServiceDiscovered);
            delete newService;
        }
    }

    // Finish off
    control.disconnectFromDevice();
    QTRY_VERIFY_WITH_TIMEOUT(
                control.state() == QLowEnergyControllerNew::UnconnectedState,
                10000);

    if (wasError) {
        QCOMPARE(disconnectedSpy.count(), 0);
    } else {
        QCOMPARE(disconnectedSpy.count(), 1);
        // after disconnect all service references must be invalid
        foreach (const QLowEnergyService *entry, savedReferences) {
            const QBluetoothUuid &uuid = entry->serviceUuid();
            QVERIFY2(entry->state() == QLowEnergyService::InvalidService,
                     uuid.toString().toLatin1());

            //after disconnect all related characteristics and descriptors are invalid
            QList<QLowEnergyCharacteristic> chars = entry->characteristics();
            for (int i = 0; i < chars.count(); i++) {
                QCOMPARE(chars.at(i).isValid(), false);
                QList<QLowEnergyDescriptor> descriptors = chars[i].descriptors();
                for (int j = 0; j < descriptors.count(); j++)
                    QCOMPARE(descriptors[j].isValid(), false);
            }
        }
    }

    qDeleteAll(savedReferences);
    savedReferences.clear();
}

void tst_QLowEnergyController::tst_concurrentDiscovery()
{
    QList<QBluetoothHostInfo> localAdapters = QBluetoothLocalDevice::allDevices();
    if (localAdapters.isEmpty() || remoteDevice.isNull())
        QSKIP("No local Bluetooth or remote BTLE device found. Skipping test.");

    // quick setup - more elaborate test is done by connectNew()
    QLowEnergyControllerNew control(remoteDevice);
    QCOMPARE(control.state(), QLowEnergyControllerNew::UnconnectedState);
    QCOMPARE(control.error(), QLowEnergyControllerNew::NoError);

    control.connectToDevice();
    {
        QTRY_IMPL(control.state() != QLowEnergyControllerNew::ConnectingState,
              30000);
    }

    if (control.state() == QLowEnergyControllerNew::ConnectingState
            || control.error() != QLowEnergyControllerNew::NoError) {
        // default BTLE backend forever hangs in ConnectingState
        QSKIP("Cannot connect to remote device");
    }

    QCOMPARE(control.state(), QLowEnergyControllerNew::ConnectedState);

    // 2. new controller to same device fails
    {
        QLowEnergyControllerNew control2(remoteDevice);
        control2.connectToDevice();
        {
            QTRY_IMPL(control2.state() != QLowEnergyControllerNew::ConnectingState,
                      30000);
        }

        QVERIFY(control2.error() != QLowEnergyControllerNew::NoError);
    }

    /* We are testing that we can run service discovery on the same device
     * for multiple services at the same time.
     * */

    QSignalSpy discoveryFinishedSpy(&control, SIGNAL(discoveryFinished()));
    control.discoverServices();
    QTRY_VERIFY_WITH_TIMEOUT(discoveryFinishedSpy.count() == 1, 10000);

    // pick MAX_SERVICES_SAME_TIME_ACCESS services
    // and discover them at the same time
#define MAX_SERVICES_SAME_TIME_ACCESS 3
    QLowEnergyService *services[MAX_SERVICES_SAME_TIME_ACCESS];

    QVERIFY(control.services().count() >= MAX_SERVICES_SAME_TIME_ACCESS);

    QList<QBluetoothUuid> uuids = control.services();

    // initialize services
    for (int i = 0; i<MAX_SERVICES_SAME_TIME_ACCESS; i++) {
        services[i] = control.createServiceObject(uuids.at(i), this);
        QVERIFY(services[i]);
    }

    // start complete discovery
    for (int i = 0; i<MAX_SERVICES_SAME_TIME_ACCESS; i++)
        services[i]->discoverDetails();

    // wait until discovery done
    for (int i = 0; i<MAX_SERVICES_SAME_TIME_ACCESS; i++) {
        qWarning() << "Waiting for" << i << services[i]->serviceUuid();
        QTRY_VERIFY_WITH_TIMEOUT(
            services[i]->state() == QLowEnergyService::ServiceDiscovered,
            30000);
    }

    // verify discovered services
    for (int i = 0; i<MAX_SERVICES_SAME_TIME_ACCESS; i++)
        verifyServiceProperties(services[i]);

    control.disconnectFromDevice();
    QTRY_VERIFY_WITH_TIMEOUT(control.state() == QLowEnergyControllerNew::UnconnectedState,
                             30000);
    discoveryFinishedSpy.clear();

    // redo the discovery with same controller
    QLowEnergyService *services_second[MAX_SERVICES_SAME_TIME_ACCESS];
    control.connectToDevice();
    {
        QTRY_IMPL(control.state() != QLowEnergyControllerNew::ConnectingState,
              30000);
    }

    QCOMPARE(control.state(), QLowEnergyControllerNew::ConnectedState);
    control.discoverServices();
    QTRY_VERIFY_WITH_TIMEOUT(discoveryFinishedSpy.count() == 1, 10000);

    // get all details
    for (int i = 0; i<MAX_SERVICES_SAME_TIME_ACCESS; i++) {
        services_second[i] = control.createServiceObject(uuids.at(i), this);
        QVERIFY(services_second[i]->parent() == this);
        QVERIFY(services[i]);
        QVERIFY(services_second[i]->state() == QLowEnergyService::DiscoveryRequired);
        services_second[i]->discoverDetails();
    }

    // wait until discovery done
    for (int i = 0; i<MAX_SERVICES_SAME_TIME_ACCESS; i++) {
        qWarning() << "Waiting for" << i << services_second[i]->serviceUuid();
        QTRY_VERIFY_WITH_TIMEOUT(
            services_second[i]->state() == QLowEnergyService::ServiceDiscovered,
            30000);
    }

    // verify discovered services (1st and 2nd round)
    for (int i = 0; i<MAX_SERVICES_SAME_TIME_ACCESS; i++) {
        verifyServiceProperties(services_second[i]);
        //after disconnect all related characteristics and descriptors are invalid
        const QList<QLowEnergyCharacteristic> chars = services[i]->characteristics();
        for (int j = 0; j < chars.count(); j++) {
            QCOMPARE(chars.at(j).isValid(), false);
            const QList<QLowEnergyDescriptor> descriptors = chars[j].descriptors();
            for (int k = 0; k < descriptors.count(); k++)
                QCOMPARE(descriptors[k].isValid(), false);
        }

        QCOMPARE(services[i]->serviceUuid(), services_second[i]->serviceUuid());
        QCOMPARE(services[i]->serviceName(), services_second[i]->serviceName());
        QCOMPARE(services[i]->type(), services_second[i]->type());
        QVERIFY(services[i]->state() == QLowEnergyService::InvalidService);
        QVERIFY(services_second[i]->state() == QLowEnergyService::ServiceDiscovered);
    }

    // cleanup
    for (int i = 0; i<MAX_SERVICES_SAME_TIME_ACCESS; i++) {
        delete services[i];
        delete services_second[i];
    }

    control.disconnectFromDevice();
}

void tst_QLowEnergyController::verifyServiceProperties(
        const QLowEnergyService *info)
{
    if (info->serviceUuid() ==
            QBluetoothUuid(QString("00001800-0000-1000-8000-00805f9b34fb"))) {
        qDebug() << "Verifying GAP Service";
        QList<QLowEnergyCharacteristic> chars = info->characteristics();
        QCOMPARE(chars.count(), 5);

        // Device Name
        QString temp("00002a00-0000-1000-8000-00805f9b34fb");
        qDebug() << chars[0].uuid().toString() << temp;
        QCOMPARE(chars[0].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[0].handle(), 0x3u);
        QCOMPARE(chars[0].properties(), QLowEnergyCharacteristic::Read);
        QCOMPARE(chars[0].value(), QByteArray("544920424c452053656e736f7220546167"));
        QVERIFY(chars[0].isValid());
        QCOMPARE(chars[0].descriptors().count(), 0);

        // Appearance
        temp = QString("00002a01-0000-1000-8000-00805f9b34fb");
        QCOMPARE(chars[1].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[1].handle(), 0x5u);
        QCOMPARE(chars[1].properties(), QLowEnergyCharacteristic::Read);
        QCOMPARE(chars[1].value(), QByteArray("0000"));
        QVERIFY(chars[1].isValid());
        QCOMPARE(chars[1].descriptors().count(), 0);

        // Peripheral Privacy Flag
        temp = QString("00002a02-0000-1000-8000-00805f9b34fb");
        QCOMPARE(chars[2].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[2].handle(), 0x7u);
        QCOMPARE(chars[2].properties(),
                 (QLowEnergyCharacteristic::Read|QLowEnergyCharacteristic::Write));
        QCOMPARE(chars[2].value(), QByteArray("00"));
        QVERIFY(chars[2].isValid());
        QCOMPARE(chars[2].descriptors().count(), 0);

        // Reconnection Address
        temp = QString("00002a03-0000-1000-8000-00805f9b34fb");
        QCOMPARE(chars[3].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[3].handle(), 0x9u);
        QCOMPARE(chars[3].properties(),
                 (QLowEnergyCharacteristic::Read|QLowEnergyCharacteristic::Write));
        QCOMPARE(chars[3].value(), QByteArray("000000000000"));
        QVERIFY(chars[3].isValid());
        QCOMPARE(chars[3].descriptors().count(), 0);

        // Peripheral Preferred Connection Parameters
        temp = QString("00002a04-0000-1000-8000-00805f9b34fb");
        QCOMPARE(chars[4].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[4].handle(), 0xbu);
        QCOMPARE(chars[4].properties(), QLowEnergyCharacteristic::Read);
        QCOMPARE(chars[4].value(), QByteArray("5000a0000000e803"));
        QVERIFY(chars[4].isValid());
        QCOMPARE(chars[4].descriptors().count(), 0);
    } else if (info->serviceUuid() ==
                QBluetoothUuid(QString("00001801-0000-1000-8000-00805f9b34fb"))) {
        qDebug() << "Verifying GATT Service";
        QList<QLowEnergyCharacteristic> chars = info->characteristics();
        QCOMPARE(chars.count(), 1);

        // Service Changed
        QString temp("00002a05-0000-1000-8000-00805f9b34fb");
        //this should really be readable according to GATT Service spec
        QCOMPARE(chars[0].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[0].handle(), 0xeu);
        QCOMPARE(chars[0].properties(), QLowEnergyCharacteristic::Indicate);
        QCOMPARE(chars[0].value(), QByteArray(""));
        QVERIFY(chars[0].isValid());

        QCOMPARE(chars[0].descriptors().count(), 1);
        QCOMPARE(chars[0].descriptors().at(0).isValid(), true);
        QCOMPARE(chars[0].descriptors().at(0).handle(), 0xfu);
        QCOMPARE(chars[0].descriptors().at(0).uuid(),
                QBluetoothUuid(QBluetoothUuid::ClientCharacteristicConfiguration));
        QCOMPARE(chars[0].descriptors().at(0).type(),
                QBluetoothUuid::ClientCharacteristicConfiguration);
        QCOMPARE(chars[0].descriptors().at(0).value(), QByteArray("0000"));
    } else if (info->serviceUuid() ==
                QBluetoothUuid(QString("0000180a-0000-1000-8000-00805f9b34fb"))) {
        qDebug() << "Verifying Device Information";
        QList<QLowEnergyCharacteristic> chars = info->characteristics();
        QCOMPARE(chars.count(), 9);

        // System ID
        QString temp("00002a23-0000-1000-8000-00805f9b34fb");
        //this should really be readable according to GATT Service spec
        QCOMPARE(chars[0].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[0].handle(), 0x12u);
        QCOMPARE(chars[0].properties(), QLowEnergyCharacteristic::Read);
        QCOMPARE(chars[0].value(), QByteArray("6e41ab0000296abc"));
        QVERIFY(chars[0].isValid());
        QCOMPARE(chars[0].descriptors().count(), 0);

        // Model Number
        temp = QString("00002a24-0000-1000-8000-00805f9b34fb");
        QCOMPARE(chars[1].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[1].handle(), 0x14u);
        QCOMPARE(chars[1].properties(), QLowEnergyCharacteristic::Read);
        QCOMPARE(chars[1].value(), QByteArray("4e2e412e00"));
        QVERIFY(chars[1].isValid());
        QCOMPARE(chars[1].descriptors().count(), 0);

        // Serial Number
        temp = QString("00002a25-0000-1000-8000-00805f9b34fb");
        QCOMPARE(chars[2].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[2].handle(), 0x16u);
        QCOMPARE(chars[2].properties(),
                 (QLowEnergyCharacteristic::Read));
        QCOMPARE(chars[2].value(), QByteArray("4e2e412e00"));
        QVERIFY(chars[2].isValid());
        QCOMPARE(chars[2].descriptors().count(), 0);

        // Firmware Revision
        temp = QString("00002a26-0000-1000-8000-00805f9b34fb");
        QCOMPARE(chars[3].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[3].handle(), 0x18u);
        QCOMPARE(chars[3].properties(),
                 (QLowEnergyCharacteristic::Read));
        QCOMPARE(chars[3].value(), QByteArray("312e3031202846656220203720323031332900"));
        QVERIFY(chars[3].isValid());
        QCOMPARE(chars[3].descriptors().count(), 0);

        // Hardware Revision
        temp = QString("00002a27-0000-1000-8000-00805f9b34fb");
        QCOMPARE(chars[4].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[4].handle(), 0x1au);
        QCOMPARE(chars[4].properties(),
                 (QLowEnergyCharacteristic::Read));
        QCOMPARE(chars[4].value(), QByteArray("4e2e412e00"));
        QVERIFY(chars[4].isValid());
        QCOMPARE(chars[4].descriptors().count(), 0);

        // Software Revision
        temp = QString("00002a28-0000-1000-8000-00805f9b34fb");
        QCOMPARE(chars[5].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[5].handle(), 0x1cu);
        QCOMPARE(chars[5].properties(),
                 (QLowEnergyCharacteristic::Read));
        QCOMPARE(chars[5].value(), QByteArray("4e2e412e00"));
        QVERIFY(chars[5].isValid());
        QCOMPARE(chars[5].descriptors().count(), 0);

        // Manufacturer Name
        temp = QString("00002a29-0000-1000-8000-00805f9b34fb");
        QCOMPARE(chars[6].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[6].handle(), 0x1eu);
        QCOMPARE(chars[6].properties(),
                 (QLowEnergyCharacteristic::Read));
        QCOMPARE(chars[6].value(), QByteArray("546578617320496e737472756d656e747300"));
        QVERIFY(chars[6].isValid());
        QCOMPARE(chars[6].descriptors().count(), 0);

        // IEEE
        temp = QString("00002a2a-0000-1000-8000-00805f9b34fb");
        QCOMPARE(chars[7].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[7].handle(), 0x20u);
        QCOMPARE(chars[7].properties(),
                 (QLowEnergyCharacteristic::Read));
        QCOMPARE(chars[7].value(), QByteArray("fe006578706572696d656e74616c"));
        QVERIFY(chars[7].isValid());
        QCOMPARE(chars[7].descriptors().count(), 0);

        // PnP ID
        temp = QString("00002a50-0000-1000-8000-00805f9b34fb");
        QCOMPARE(chars[8].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[8].handle(), 0x22u);
        QCOMPARE(chars[8].properties(),
                 (QLowEnergyCharacteristic::Read));
        QCOMPARE(chars[8].value(), QByteArray("010d0000001001"));
        QVERIFY(chars[8].isValid());
        QCOMPARE(chars[8].descriptors().count(), 0);
    } else if (info->serviceUuid() ==
               QBluetoothUuid(QString("f000aa00-0451-4000-b000-000000000000"))) {
        qDebug() << "Verifying Temperature";
        QList<QLowEnergyCharacteristic> chars = info->characteristics();
        QCOMPARE(chars.count(), 2);

        // Temp Data
        QString temp("f000aa01-0451-4000-b000-000000000000");
        QCOMPARE(chars[0].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[0].handle(), 0x25u);
        QCOMPARE(chars[0].properties(),
                (QLowEnergyCharacteristic::Read|QLowEnergyCharacteristic::Notify));
        QCOMPARE(chars[0].value(), QByteArray("00000000"));
        QVERIFY(chars[0].isValid());

        QCOMPARE(chars[0].descriptors().count(), 2);
        //descriptor checks
        QCOMPARE(chars[0].descriptors().at(0).isValid(), true);
        QCOMPARE(chars[0].descriptors().at(0).handle(), 0x26u);
        QCOMPARE(chars[0].descriptors().at(0).uuid(),
                QBluetoothUuid(QBluetoothUuid::ClientCharacteristicConfiguration));
        QCOMPARE(chars[0].descriptors().at(0).type(),
                QBluetoothUuid::ClientCharacteristicConfiguration);
        QCOMPARE(chars[0].descriptors().at(0).value(), QByteArray("0000"));

        QCOMPARE(chars[0].descriptors().at(1).isValid(), true);
        QCOMPARE(chars[0].descriptors().at(1).handle(), 0x27u);
        QCOMPARE(chars[0].descriptors().at(1).uuid(),
                QBluetoothUuid(QBluetoothUuid::CharacteristicUserDescription));
        QCOMPARE(chars[0].descriptors().at(1).type(),
                QBluetoothUuid::CharacteristicUserDescription);
        QCOMPARE(chars[0].descriptors().at(1).value(),
                QByteArray("49522054656d702e2044617461"));

        // Temp Config
        temp = QString("f000aa02-0451-4000-b000-000000000000");
        QCOMPARE(chars[1].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[1].handle(), 0x29u);
        QCOMPARE(chars[1].properties(),
                 (QLowEnergyCharacteristic::Read|QLowEnergyCharacteristic::Write));
        QCOMPARE(chars[1].value(), QByteArray("00"));
        QVERIFY(chars[1].isValid());

        QCOMPARE(chars[1].descriptors().count(), 1);
        //descriptor checks
        QCOMPARE(chars[1].descriptors().at(0).isValid(), true);
        QCOMPARE(chars[1].descriptors().at(0).handle(), 0x2au);
        QCOMPARE(chars[1].descriptors().at(0).uuid(),
                QBluetoothUuid(QBluetoothUuid::CharacteristicUserDescription));
        QCOMPARE(chars[1].descriptors().at(0).type(),
                QBluetoothUuid::CharacteristicUserDescription);
        QCOMPARE(chars[1].descriptors().at(0).value(),
                QByteArray("49522054656d702e20436f6e662e"));
    } else if (info->serviceUuid() ==
               QBluetoothUuid(QString("0000ffe0-0000-1000-8000-00805f9b34fb"))) {
        qDebug() << "Verifying Simple Keys";
        QList<QLowEnergyCharacteristic> chars = info->characteristics();
        QCOMPARE(chars.count(), 1);

        // Temp Data
        QString temp("0000ffe1-0000-1000-8000-00805f9b34fb");
        QCOMPARE(chars[0].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[0].handle(), 0x5fu);
        QCOMPARE(chars[0].properties(),
                (QLowEnergyCharacteristic::Notify));
        QCOMPARE(chars[0].value(), QByteArray(""));
        QVERIFY(chars[0].isValid());

        QCOMPARE(chars[0].descriptors().count(), 2);
        //descriptor checks
        QCOMPARE(chars[0].descriptors().at(0).isValid(), true);
        QCOMPARE(chars[0].descriptors().at(0).handle(), 0x60u);
        QCOMPARE(chars[0].descriptors().at(0).uuid(),
                QBluetoothUuid(QBluetoothUuid::ClientCharacteristicConfiguration));
        QCOMPARE(chars[0].descriptors().at(0).type(),
                QBluetoothUuid::ClientCharacteristicConfiguration);
        QCOMPARE(chars[0].descriptors().at(0).value(), QByteArray("0000"));

        QCOMPARE(chars[0].descriptors().at(1).isValid(), true);
        QCOMPARE(chars[0].descriptors().at(1).handle(), 0x61u);
        QCOMPARE(chars[0].descriptors().at(1).uuid(),
                QBluetoothUuid(QBluetoothUuid::CharacteristicUserDescription));
        QCOMPARE(chars[0].descriptors().at(1).type(),
                QBluetoothUuid::CharacteristicUserDescription);
        QCOMPARE(chars[0].descriptors().at(1).value(),
                QByteArray("4b6579205072657373205374617465"));

    } else if (info->serviceUuid() ==
               QBluetoothUuid(QString("f000aa10-0451-4000-b000-000000000000"))) {
        qDebug() << "Verifying Accelerometer";
        QList<QLowEnergyCharacteristic> chars = info->characteristics();
        QCOMPARE(chars.count(), 3);

        // Accel Data
        QString temp("f000aa11-0451-4000-b000-000000000000");
        QCOMPARE(chars[0].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[0].handle(), 0x2du);
        QCOMPARE(chars[0].properties(),
                (QLowEnergyCharacteristic::Read|QLowEnergyCharacteristic::Notify));
        QCOMPARE(chars[0].value(), QByteArray("000000"));
        QVERIFY(chars[0].isValid());

        QCOMPARE(chars[0].descriptors().count(), 2);

        QCOMPARE(chars[0].descriptors().at(0).isValid(), true);
        QCOMPARE(chars[0].descriptors().at(0).handle(), 0x2eu);
        QCOMPARE(chars[0].descriptors().at(0).uuid(),
                 QBluetoothUuid(QBluetoothUuid::ClientCharacteristicConfiguration));
        QCOMPARE(chars[0].descriptors().at(0).type(),
                 QBluetoothUuid::ClientCharacteristicConfiguration);
        QCOMPARE(chars[0].descriptors().at(0).value(), QByteArray("0000"));

        QCOMPARE(chars[0].descriptors().at(1).isValid(), true);
        QCOMPARE(chars[0].descriptors().at(1).handle(), 0x2fu);
        QCOMPARE(chars[0].descriptors().at(1).uuid(),
                 QBluetoothUuid(QBluetoothUuid::CharacteristicUserDescription));
        QCOMPARE(chars[0].descriptors().at(1).type(),
                 QBluetoothUuid::CharacteristicUserDescription);
        QCOMPARE(chars[0].descriptors().at(1).value(),
                 QByteArray("416363656c2e2044617461"));

        // Accel Config
        temp = QString("f000aa12-0451-4000-b000-000000000000");
        QCOMPARE(chars[1].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[1].handle(), 0x31u);
        QCOMPARE(chars[1].properties(),
                 (QLowEnergyCharacteristic::Read|QLowEnergyCharacteristic::Write));
        QCOMPARE(chars[1].value(), QByteArray("00"));
        QVERIFY(chars[1].isValid());
        QCOMPARE(chars[1].descriptors().count(), 1);

        QCOMPARE(chars[1].descriptors().at(0).isValid(), true);
        QCOMPARE(chars[1].descriptors().at(0).handle(), 0x32u);
        QCOMPARE(chars[1].descriptors().at(0).uuid(),
                 QBluetoothUuid(QBluetoothUuid::CharacteristicUserDescription));
        QCOMPARE(chars[1].descriptors().at(0).type(),
                 QBluetoothUuid::CharacteristicUserDescription);
        QCOMPARE(chars[1].descriptors().at(0).value(),
                 QByteArray("416363656c2e20436f6e662e"));

        // Accel Period
        temp = QString("f000aa13-0451-4000-b000-000000000000");
        QCOMPARE(chars[2].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[2].handle(), 0x34u);
        QCOMPARE(chars[2].properties(),
                 (QLowEnergyCharacteristic::Read|QLowEnergyCharacteristic::Write));
        QCOMPARE(chars[2].value(), QByteArray("64"));   // don't change it or set it to 0x64
        QVERIFY(chars[2].isValid());

        QCOMPARE(chars[2].descriptors().count(), 1);
        //descriptor checks
        QCOMPARE(chars[2].descriptors().at(0).isValid(), true);
        QCOMPARE(chars[2].descriptors().at(0).handle(), 0x35u);
        QCOMPARE(chars[2].descriptors().at(0).uuid(),
                QBluetoothUuid(QBluetoothUuid::CharacteristicUserDescription));
        QCOMPARE(chars[2].descriptors().at(0).type(),
                QBluetoothUuid::CharacteristicUserDescription);
        QCOMPARE(chars[2].descriptors().at(0).value(),
                QByteArray("4163632e20506572696f64"));
    } else if (info->serviceUuid() ==
               QBluetoothUuid(QString("f000aa20-0451-4000-b000-000000000000"))) {
        qDebug() << "Verifying Humidity";
        QList<QLowEnergyCharacteristic> chars = info->characteristics();
        QCOMPARE(chars.count(), 2);

        // Humidity Data
        QString temp("f000aa21-0451-4000-b000-000000000000");
        QCOMPARE(chars[0].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[0].handle(), 0x38u);
        QCOMPARE(chars[0].properties(),
                (QLowEnergyCharacteristic::Read|QLowEnergyCharacteristic::Notify));
        QCOMPARE(chars[0].value(), QByteArray("00000000"));
        QVERIFY(chars[0].isValid());

        QCOMPARE(chars[0].descriptors().count(), 2);
        //descriptor checks
        QCOMPARE(chars[0].descriptors().at(0).isValid(), true);
        QCOMPARE(chars[0].descriptors().at(0).handle(), 0x39u);
        QCOMPARE(chars[0].descriptors().at(0).uuid(),
                QBluetoothUuid(QBluetoothUuid::ClientCharacteristicConfiguration));
        QCOMPARE(chars[0].descriptors().at(0).type(),
                QBluetoothUuid::ClientCharacteristicConfiguration);
        QCOMPARE(chars[0].descriptors().at(0).value(), QByteArray("0000"));

        QCOMPARE(chars[0].descriptors().at(1).isValid(), true);
        QCOMPARE(chars[0].descriptors().at(1).handle(), 0x3au);
        QCOMPARE(chars[0].descriptors().at(1).uuid(),
                QBluetoothUuid(QBluetoothUuid::CharacteristicUserDescription));
        QCOMPARE(chars[0].descriptors().at(1).type(),
                QBluetoothUuid::CharacteristicUserDescription);
        QCOMPARE(chars[0].descriptors().at(1).value(),
                QByteArray("48756d69642e2044617461"));

        // Humidity Config
        temp = QString("f000aa22-0451-4000-b000-000000000000");
        QCOMPARE(chars[1].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[1].handle(), 0x3cu);
        QCOMPARE(chars[1].properties(),
                 (QLowEnergyCharacteristic::Read|QLowEnergyCharacteristic::Write));
        QCOMPARE(chars[1].value(), QByteArray("00"));
        QVERIFY(chars[1].isValid());

        QCOMPARE(chars[1].descriptors().count(), 1);
        //descriptor checks
        QCOMPARE(chars[1].descriptors().at(0).isValid(), true);
        QCOMPARE(chars[1].descriptors().at(0).handle(), 0x3du);
        QCOMPARE(chars[1].descriptors().at(0).uuid(),
                QBluetoothUuid(QBluetoothUuid::CharacteristicUserDescription));
        QCOMPARE(chars[1].descriptors().at(0).type(),
                QBluetoothUuid::CharacteristicUserDescription);
        QCOMPARE(chars[1].descriptors().at(0).value(),
                QByteArray("48756d69642e20436f6e662e"));
    } else if (info->serviceUuid() ==
               QBluetoothUuid(QString("f000aa30-0451-4000-b000-000000000000"))) {
        qDebug() << "Verifying Magnetometer";
        QList<QLowEnergyCharacteristic> chars = info->characteristics();
        QCOMPARE(chars.count(), 3);

        // Magnetometer Data
        QString temp("f000aa31-0451-4000-b000-000000000000");
        QCOMPARE(chars[0].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[0].handle(), 0x40u);
        QCOMPARE(chars[0].properties(),
                (QLowEnergyCharacteristic::Read|QLowEnergyCharacteristic::Notify));
        QCOMPARE(chars[0].value(), QByteArray("000000000000"));
        QVERIFY(chars[0].isValid());

        QCOMPARE(chars[0].descriptors().count(), 2);

        QCOMPARE(chars[0].descriptors().at(0).isValid(), true);
        QCOMPARE(chars[0].descriptors().at(0).handle(), 0x41u);
        QCOMPARE(chars[0].descriptors().at(0).uuid(),
                 QBluetoothUuid(QBluetoothUuid::ClientCharacteristicConfiguration));
        QCOMPARE(chars[0].descriptors().at(0).type(),
                 QBluetoothUuid::ClientCharacteristicConfiguration);
        QCOMPARE(chars[0].descriptors().at(0).value(), QByteArray("0000"));

        QCOMPARE(chars[0].descriptors().at(1).isValid(), true);
        QCOMPARE(chars[0].descriptors().at(1).handle(), 0x42u);
        QCOMPARE(chars[0].descriptors().at(1).uuid(),
                 QBluetoothUuid(QBluetoothUuid::CharacteristicUserDescription));
        QCOMPARE(chars[0].descriptors().at(1).type(),
                 QBluetoothUuid::CharacteristicUserDescription);
        QCOMPARE(chars[0].descriptors().at(1).value(),
                 QByteArray("4d61672e2044617461"));

        // Magnetometer Config
        temp = QString("f000aa32-0451-4000-b000-000000000000");
        QCOMPARE(chars[1].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[1].handle(), 0x44u);
        QCOMPARE(chars[1].properties(),
                 (QLowEnergyCharacteristic::Read|QLowEnergyCharacteristic::Write));
        QCOMPARE(chars[1].value(), QByteArray("00"));
        QVERIFY(chars[1].isValid());

        QCOMPARE(chars[1].descriptors().count(), 1);
        QCOMPARE(chars[1].descriptors().at(0).isValid(), true);
        QCOMPARE(chars[1].descriptors().at(0).handle(), 0x45u);
        QCOMPARE(chars[1].descriptors().at(0).uuid(),
                 QBluetoothUuid(QBluetoothUuid::CharacteristicUserDescription));
        QCOMPARE(chars[1].descriptors().at(0).type(),
                 QBluetoothUuid::CharacteristicUserDescription);
        QCOMPARE(chars[1].descriptors().at(0).value(),
                 QByteArray("4d61672e20436f6e662e"));

        // Magnetometer Period
        temp = QString("f000aa33-0451-4000-b000-000000000000");
        QCOMPARE(chars[2].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[2].handle(), 0x47u);
        QCOMPARE(chars[2].properties(),
                 (QLowEnergyCharacteristic::Read|QLowEnergyCharacteristic::Write));
        QCOMPARE(chars[2].value(), QByteArray("c8"));   // don't change it or set it to 0xc8
        QVERIFY(chars[2].isValid());

        QCOMPARE(chars[2].descriptors().count(), 1);
        QCOMPARE(chars[2].descriptors().at(0).isValid(), true);
        QCOMPARE(chars[2].descriptors().at(0).handle(), 0x48u);
        QCOMPARE(chars[2].descriptors().at(0).uuid(),
                 QBluetoothUuid(QBluetoothUuid::CharacteristicUserDescription));
        QCOMPARE(chars[2].descriptors().at(0).type(),
                 QBluetoothUuid::CharacteristicUserDescription);
        QCOMPARE(chars[2].descriptors().at(0).value(),
                 QByteArray("4d61672e20506572696f64"));
    } else if (info->serviceUuid() ==
               QBluetoothUuid(QString("f000aa40-0451-4000-b000-000000000000"))) {
        qDebug() << "Verifying Pressure";
        QList<QLowEnergyCharacteristic> chars = info->characteristics();
        QCOMPARE(chars.count(), 3);

        // Pressure Data
        QString temp("f000aa41-0451-4000-b000-000000000000");
        QCOMPARE(chars[0].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[0].handle(), 0x4bu);
        QCOMPARE(chars[0].properties(),
                (QLowEnergyCharacteristic::Read|QLowEnergyCharacteristic::Notify));
        QCOMPARE(chars[0].value(), QByteArray("00000000"));
        QVERIFY(chars[0].isValid());

        QCOMPARE(chars[0].descriptors().count(), 2);
        //descriptor checks
        QCOMPARE(chars[0].descriptors().at(0).isValid(), true);
        QCOMPARE(chars[0].descriptors().at(0).handle(), 0x4cu);
        QCOMPARE(chars[0].descriptors().at(0).uuid(),
                QBluetoothUuid(QBluetoothUuid::ClientCharacteristicConfiguration));
        QCOMPARE(chars[0].descriptors().at(0).type(),
                QBluetoothUuid::ClientCharacteristicConfiguration);
        QCOMPARE(chars[0].descriptors().at(0).value(), QByteArray("0000"));

        QCOMPARE(chars[0].descriptors().at(1).isValid(), true);
        QCOMPARE(chars[0].descriptors().at(1).handle(), 0x4du);
        QCOMPARE(chars[0].descriptors().at(1).uuid(),
                QBluetoothUuid(QBluetoothUuid::CharacteristicUserDescription));
        QCOMPARE(chars[0].descriptors().at(1).type(),
                QBluetoothUuid::CharacteristicUserDescription);
        QCOMPARE(chars[0].descriptors().at(1).value(),
                QByteArray("4261726f6d657465722044617461"));

        // Pressure Config
        temp = QString("f000aa42-0451-4000-b000-000000000000");
        QCOMPARE(chars[1].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[1].handle(), 0x4fu);
        QCOMPARE(chars[1].properties(),
                 (QLowEnergyCharacteristic::Read|QLowEnergyCharacteristic::Write));
        QCOMPARE(chars[1].value(), QByteArray("00"));
        QVERIFY(chars[1].isValid());

        QCOMPARE(chars[1].descriptors().count(), 1);
        QCOMPARE(chars[1].descriptors().at(0).isValid(), true);
        QCOMPARE(chars[1].descriptors().at(0).handle(), 0x50u);
        QCOMPARE(chars[1].descriptors().at(0).uuid(),
                QBluetoothUuid(QBluetoothUuid::CharacteristicUserDescription));
        QCOMPARE(chars[1].descriptors().at(0).type(),
                QBluetoothUuid::CharacteristicUserDescription);
        QCOMPARE(chars[1].descriptors().at(0).value(),
                QByteArray("4261726f6d6574657220436f6e662e"));

        // Pressure Calibration
        temp = QString("f000aa43-0451-4000-b000-000000000000");
        QCOMPARE(chars[2].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[2].handle(), 0x52u);
        QCOMPARE(chars[2].properties(),
                 (QLowEnergyCharacteristic::Read));
        QCOMPARE(chars[2].value(), QByteArray("00000000000000000000000000000000"));   // don't change it
        QVERIFY(chars[2].isValid());

        QCOMPARE(chars[2].descriptors().count(), 2);
        //descriptor checks
        QCOMPARE(chars[2].descriptors().at(0).isValid(), true);
        QCOMPARE(chars[2].descriptors().at(0).handle(), 0x53u);
        QCOMPARE(chars[2].descriptors().at(0).uuid(),
                QBluetoothUuid(QBluetoothUuid::ClientCharacteristicConfiguration));
        QCOMPARE(chars[2].descriptors().at(0).type(),
                QBluetoothUuid::ClientCharacteristicConfiguration);
        QCOMPARE(chars[2].descriptors().at(0).value(), QByteArray("0000"));

        QCOMPARE(chars[2].descriptors().at(1).isValid(), true);
        QCOMPARE(chars[2].descriptors().at(1).handle(), 0x54u);
        QCOMPARE(chars[2].descriptors().at(1).uuid(),
                QBluetoothUuid(QBluetoothUuid::CharacteristicUserDescription));
        QCOMPARE(chars[2].descriptors().at(1).type(),
                QBluetoothUuid::CharacteristicUserDescription);
        QCOMPARE(chars[2].descriptors().at(1).value(), QByteArray("4261726f6d657465722043616c692e"));
    } else if (info->serviceUuid() ==
               QBluetoothUuid(QString("f000aa50-0451-4000-b000-000000000000"))) {
        qDebug() << "Verifying Gyroscope";
        QList<QLowEnergyCharacteristic> chars = info->characteristics();
        QCOMPARE(chars.count(), 2);

        // Gyroscope Data
        QString temp("f000aa51-0451-4000-b000-000000000000");
        QCOMPARE(chars[0].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[0].handle(), 0x57u);
        QCOMPARE(chars[0].properties(),
                (QLowEnergyCharacteristic::Read|QLowEnergyCharacteristic::Notify));
        QCOMPARE(chars[0].value(), QByteArray("000000000000"));
        QVERIFY(chars[0].isValid());

        QCOMPARE(chars[0].descriptors().count(), 2);
        //descriptor checks
        QCOMPARE(chars[0].descriptors().at(0).isValid(), true);
        QCOMPARE(chars[0].descriptors().at(0).handle(), 0x58u);
        QCOMPARE(chars[0].descriptors().at(0).uuid(),
                QBluetoothUuid(QBluetoothUuid::ClientCharacteristicConfiguration));
        QCOMPARE(chars[0].descriptors().at(0).type(),
                QBluetoothUuid::ClientCharacteristicConfiguration);
        QCOMPARE(chars[0].descriptors().at(0).value(), QByteArray("0000"));

        QCOMPARE(chars[0].descriptors().at(1).isValid(), true);
        QCOMPARE(chars[0].descriptors().at(1).handle(), 0x59u);
        QCOMPARE(chars[0].descriptors().at(1).uuid(),
                QBluetoothUuid(QBluetoothUuid::CharacteristicUserDescription));
        QCOMPARE(chars[0].descriptors().at(1).type(),
                QBluetoothUuid::CharacteristicUserDescription);
        QCOMPARE(chars[0].descriptors().at(1).value(),
                QByteArray("4779726f2e2044617461"));

        // Gyroscope Config
        temp = QString("f000aa52-0451-4000-b000-000000000000");
        QCOMPARE(chars[1].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[1].handle(), 0x5bu);
        QCOMPARE(chars[1].properties(),
                 (QLowEnergyCharacteristic::Read|QLowEnergyCharacteristic::Write));
        QCOMPARE(chars[1].value(), QByteArray("00"));
        QVERIFY(chars[1].isValid());

        QCOMPARE(chars[1].descriptors().count(), 1);
        //descriptor checks
        QCOMPARE(chars[1].descriptors().at(0).isValid(), true);
        QCOMPARE(chars[1].descriptors().at(0).handle(), 0x5cu);
        QCOMPARE(chars[1].descriptors().at(0).uuid(),
                QBluetoothUuid(QBluetoothUuid::CharacteristicUserDescription));
        QCOMPARE(chars[1].descriptors().at(0).type(),
                QBluetoothUuid::CharacteristicUserDescription);
        QCOMPARE(chars[1].descriptors().at(0).value(),
                QByteArray("4779726f2e20436f6e662e"));
    } else if (info->serviceUuid() ==
               QBluetoothUuid(QString("f000aa60-0451-4000-b000-000000000000"))) {
        qDebug() << "Verifying Test Service";
        QList<QLowEnergyCharacteristic> chars = info->characteristics();
        QCOMPARE(chars.count(), 2);

        // Test Data
        QString temp("f000aa61-0451-4000-b000-000000000000");
        QCOMPARE(chars[0].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[0].handle(), 0x64u);
        QCOMPARE(chars[0].properties(),
                (QLowEnergyCharacteristic::Read));
        QCOMPARE(chars[0].value(), QByteArray("3f00"));
        QVERIFY(chars[0].isValid());

        QCOMPARE(chars[0].descriptors().count(), 1);
        QCOMPARE(chars[0].descriptors().at(0).isValid(), true);
        QCOMPARE(chars[0].descriptors().at(0).handle(), 0x65u);
        QCOMPARE(chars[0].descriptors().at(0).uuid(),
                QBluetoothUuid(QBluetoothUuid::CharacteristicUserDescription));
        QCOMPARE(chars[0].descriptors().at(0).type(),
                QBluetoothUuid::CharacteristicUserDescription);
        QCOMPARE(chars[0].descriptors().at(0).value(),
                QByteArray("546573742044617461"));

        // Test Config
        temp = QString("f000aa62-0451-4000-b000-000000000000");
        QCOMPARE(chars[1].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[1].handle(), 0x67u);
        QCOMPARE(chars[1].properties(),
                 (QLowEnergyCharacteristic::Read|QLowEnergyCharacteristic::Write));
        QCOMPARE(chars[1].value(), QByteArray("00"));
        QVERIFY(chars[1].isValid());

        QCOMPARE(chars[1].descriptors().count(), 1);
        //descriptor checks
        QCOMPARE(chars[1].descriptors().at(0).isValid(), true);
        QCOMPARE(chars[1].descriptors().at(0).handle(), 0x68u);
        QCOMPARE(chars[1].descriptors().at(0).uuid(),
                QBluetoothUuid(QBluetoothUuid::CharacteristicUserDescription));
        QCOMPARE(chars[1].descriptors().at(0).type(),
                QBluetoothUuid::CharacteristicUserDescription);
        QCOMPARE(chars[1].descriptors().at(0).value(),
                QByteArray("5465737420436f6e666967"));
    } else if (info->serviceUuid() ==
               QBluetoothUuid(QString("f000ffc0-0451-4000-b000-000000000000"))) {
        qDebug() << "Verifying Unknown Service";
        QList<QLowEnergyCharacteristic> chars = info->characteristics();
        QCOMPARE(chars.count(), 2);

        // first characteristic
        QString temp("f000ffc1-0451-4000-b000-000000000000");
        QCOMPARE(chars[0].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[0].handle(), 0x6bu);
        QCOMPARE(chars[0].properties(),
                (QLowEnergyCharacteristic::Notify|QLowEnergyCharacteristic::Write|QLowEnergyCharacteristicInfo::WriteNoResponse));
        QCOMPARE(chars[0].value(), QByteArray(""));
        QVERIFY(chars[0].isValid());

        QCOMPARE(chars[0].descriptors().count(), 2);
        //descriptor checks
        QCOMPARE(chars[0].descriptors().at(0).isValid(), true);
        QCOMPARE(chars[0].descriptors().at(0).handle(), 0x6cu);
        QCOMPARE(chars[0].descriptors().at(0).uuid(),
                QBluetoothUuid(QBluetoothUuid::ClientCharacteristicConfiguration));
        QCOMPARE(chars[0].descriptors().at(0).type(),
                QBluetoothUuid::ClientCharacteristicConfiguration);
        QCOMPARE(chars[0].descriptors().at(0).value(), QByteArray("0000"));

        QCOMPARE(chars[0].descriptors().at(1).isValid(), true);
        QCOMPARE(chars[0].descriptors().at(1).handle(), 0x6du);
        QCOMPARE(chars[0].descriptors().at(1).uuid(),
                QBluetoothUuid(QBluetoothUuid::CharacteristicUserDescription));
        QCOMPARE(chars[0].descriptors().at(1).type(),
                QBluetoothUuid::CharacteristicUserDescription);
        QCOMPARE(chars[0].descriptors().at(1).value(),
                QByteArray("496d67204964656e74696679"));

        // second characteristic
        temp = QString("f000ffc2-0451-4000-b000-000000000000");
        QCOMPARE(chars[1].uuid(), QBluetoothUuid(temp));
        QCOMPARE(chars[1].handle(), 0x6fu);
        QCOMPARE(chars[1].properties(),
                 (QLowEnergyCharacteristic::Notify|QLowEnergyCharacteristic::Write|QLowEnergyCharacteristicInfo::WriteNoResponse));
        QCOMPARE(chars[1].value(), QByteArray(""));
        QVERIFY(chars[1].isValid());

        QCOMPARE(chars[1].descriptors().count(), 2);
        //descriptor checks
        QCOMPARE(chars[1].descriptors().at(0).isValid(), true);
        QCOMPARE(chars[1].descriptors().at(0).handle(), 0x70u);
        QCOMPARE(chars[1].descriptors().at(0).uuid(),
                QBluetoothUuid(QBluetoothUuid::ClientCharacteristicConfiguration));
        QCOMPARE(chars[1].descriptors().at(0).type(),
                QBluetoothUuid::ClientCharacteristicConfiguration);
        QCOMPARE(chars[1].descriptors().at(0).value(), QByteArray("0000"));

        QCOMPARE(chars[1].descriptors().at(1).isValid(), true);
        QCOMPARE(chars[1].descriptors().at(1).handle(), 0x71u);
        QCOMPARE(chars[1].descriptors().at(1).uuid(),
                QBluetoothUuid(QBluetoothUuid::CharacteristicUserDescription));
        QCOMPARE(chars[1].descriptors().at(1).type(),
                QBluetoothUuid::CharacteristicUserDescription);
        QCOMPARE(chars[1].descriptors().at(1).value(),
                QByteArray("496d6720426c6f636b"));
    } else {
        QFAIL(QString("Service not found" + info->serviceUuid().toString()).toUtf8().constData());
    }
}

void tst_QLowEnergyController::tst_defaultBehavior()
{
    QList<QBluetoothAddress> foundAddresses;
    foreach (const QBluetoothHostInfo &info, QBluetoothLocalDevice::allDevices())
        foundAddresses.append(info.address());
    const QBluetoothAddress randomAddress("11:22:33:44:55:66");

    // Test automatic detection of local adapter
    QLowEnergyControllerNew controlDefaultAdapter(randomAddress);
    QCOMPARE(controlDefaultAdapter.remoteAddress(), randomAddress);
    QCOMPARE(controlDefaultAdapter.state(), QLowEnergyControllerNew::UnconnectedState);
    if (foundAddresses.isEmpty()) {
        QVERIFY(controlDefaultAdapter.localAddress().isNull());
    } else {
        QCOMPARE(controlDefaultAdapter.error(), QLowEnergyControllerNew::NoError);
        QVERIFY(controlDefaultAdapter.errorString().isEmpty());
        QVERIFY(foundAddresses.contains(controlDefaultAdapter.localAddress()));

        // unrelated uuids don't return valid service object
        // invalid service uuid
        QVERIFY(!controlDefaultAdapter.createServiceObject(
                    QBluetoothUuid()));
        // some random uuid
        QVERIFY(!controlDefaultAdapter.createServiceObject(
                    QBluetoothUuid(QBluetoothUuid::DeviceName)));
    }

    QCOMPARE(controlDefaultAdapter.services().count(), 0);

    // Test explicit local adapter
    if (!foundAddresses.isEmpty()) {
        QLowEnergyControllerNew controlExplicitAdapter(randomAddress,
                                                       foundAddresses[0]);
        QCOMPARE(controlExplicitAdapter.remoteAddress(), randomAddress);
        QCOMPARE(controlExplicitAdapter.localAddress(), foundAddresses[0]);
        QCOMPARE(controlExplicitAdapter.state(),
                 QLowEnergyControllerNew::UnconnectedState);
        QCOMPARE(controlExplicitAdapter.services().count(), 0);

        // unrelated uuids don't return valid service object
        // invalid service uuid
        QVERIFY(!controlExplicitAdapter.createServiceObject(
                    QBluetoothUuid()));
        // some random uuid
        QVERIFY(!controlExplicitAdapter.createServiceObject(
                    QBluetoothUuid(QBluetoothUuid::DeviceName)));
    }


}

void tst_QLowEnergyController::tst_writeCharacteristic()
{
    QList<QBluetoothHostInfo> localAdapters = QBluetoothLocalDevice::allDevices();
    if (localAdapters.isEmpty() || remoteDevice.isNull())
        QSKIP("No local Bluetooth or remote BTLE device found. Skipping test.");

    // quick setup - more elaborate test is done by connectNew()
    QLowEnergyControllerNew control(remoteDevice);
    QCOMPARE(control.state(), QLowEnergyControllerNew::UnconnectedState);
    QCOMPARE(control.error(), QLowEnergyControllerNew::NoError);

    control.connectToDevice();
    {
        QTRY_IMPL(control.state() != QLowEnergyControllerNew::ConnectingState,
              30000);
    }

    if (control.state() == QLowEnergyControllerNew::ConnectingState
            || control.error() != QLowEnergyControllerNew::NoError) {
        // default BTLE backend forever hangs in ConnectingState
        QSKIP("Cannot connect to remote device");
    }

    QCOMPARE(control.state(), QLowEnergyControllerNew::ConnectedState);
    QSignalSpy discoveryFinishedSpy(&control, SIGNAL(discoveryFinished()));
    control.discoverServices();
    QTRY_VERIFY_WITH_TIMEOUT(discoveryFinishedSpy.count() == 1, 10000);

    const QBluetoothUuid testService(QString("f000aa60-0451-4000-b000-000000000000"));
    QList<QBluetoothUuid> uuids = control.services();
    QVERIFY(uuids.contains(testService));

    QLowEnergyService *service = control.createServiceObject(testService, this);
    QVERIFY(service);
    service->discoverDetails();
    QTRY_VERIFY_WITH_TIMEOUT(
        service->state() == QLowEnergyService::ServiceDiscovered, 30000);

    //test service described by http://processors.wiki.ti.com/index.php/SensorTag_User_Guide
    const QList<QLowEnergyCharacteristic> chars = service->characteristics();

    QLowEnergyCharacteristic dataChar;
    QLowEnergyCharacteristic configChar;
    for (int i = 0; i < chars.count(); i++) {
        if (chars[i].uuid() == QBluetoothUuid(QString("f000aa61-0451-4000-b000-000000000000")))
            dataChar = chars[i];
        else if (chars[i].uuid() == QBluetoothUuid(QString("f000aa62-0451-4000-b000-000000000000")))
            configChar = chars[i];
    }

    QVERIFY(dataChar.isValid());
    QVERIFY(!(dataChar.properties() & ~QLowEnergyCharacteristic::Read)); // only a read char
    QVERIFY(configChar.isValid());
    QVERIFY(configChar.properties() & QLowEnergyCharacteristic::Write);

    QCOMPARE(dataChar.value(), QByteArray("3f00"));
    QVERIFY(configChar.value() == QByteArray("00") || configChar.value() == QByteArray("81"));

    QSignalSpy writeSpy(service,
                        SIGNAL(characteristicChanged(QLowEnergyCharacteristic,QByteArray)));

    // *******************************************
    // test writing of characteristic
    // enable Blinking LED if not already enabled
    if (configChar.value() != QByteArray("81")) {
        service->writeCharacteristic(configChar, QByteArray("81")); //0x81 blink LED D1
        QTRY_VERIFY_WITH_TIMEOUT(!writeSpy.isEmpty(), 10000);
        QCOMPARE(configChar.value(), QByteArray("81"));
        QList<QVariant> firstSignalData= writeSpy.first();
        QLowEnergyCharacteristic signalChar = firstSignalData[0].value<QLowEnergyCharacteristic>();
        QByteArray signalValue = firstSignalData[1].toByteArray();

        QCOMPARE(signalValue, QByteArray("81"));
        QVERIFY(signalChar == configChar);

        writeSpy.clear();
    }

    service->writeCharacteristic(configChar, QByteArray("00")); //0x81 blink LED D1
    QTRY_VERIFY_WITH_TIMEOUT(!writeSpy.isEmpty(), 10000);
    QCOMPARE(configChar.value(), QByteArray("00"));
    QList<QVariant> firstSignalData = writeSpy.first();
    QLowEnergyCharacteristic signalChar = firstSignalData[0].value<QLowEnergyCharacteristic>();
    QByteArray signalValue = firstSignalData[1].toByteArray();

    QCOMPARE(signalValue, QByteArray("00"));
    QVERIFY(signalChar == configChar);

    // *******************************************
    // write wrong value -> error response required
    QSignalSpy errorSpy(service, SIGNAL(error(QLowEnergyService::ServiceError)));
    writeSpy.clear();
    QCOMPARE(errorSpy.count(), 0);
    QCOMPARE(writeSpy.count(), 0);

    // write 2 byte value to 1 byte characteristic
    service->writeCharacteristic(configChar, QByteArray("1111"));
    QTRY_VERIFY_WITH_TIMEOUT(!errorSpy.isEmpty(), 10000);
    QCOMPARE(errorSpy[0].at(0).value<QLowEnergyService::ServiceError>(),
             QLowEnergyService::CharacteristicWriteError);
    QCOMPARE(service->error(), QLowEnergyService::CharacteristicWriteError);
    QCOMPARE(writeSpy.count(), 0);
    QCOMPARE(configChar.value(), QByteArray("00"));

    // *******************************************
    // write to read-only characteristic -> error
    errorSpy.clear();
    QCOMPARE(errorSpy.count(), 0);
    service->writeCharacteristic(dataChar, QByteArray("ffff"));

    QTRY_VERIFY_WITH_TIMEOUT(!errorSpy.isEmpty(), 10000);
    QCOMPARE(errorSpy[0].at(0).value<QLowEnergyService::ServiceError>(),
             QLowEnergyService::OperationError);
    QCOMPARE(service->error(), QLowEnergyService::OperationError);
    QCOMPARE(writeSpy.count(), 0);
    QCOMPARE(dataChar.value(), QByteArray("3f00"));


    control.disconnectFromDevice();

    // *******************************************
    // write value while disconnected -> error
    errorSpy.clear();
    QCOMPARE(errorSpy.count(), 0);
    service->writeCharacteristic(configChar, QByteArray("ffff"));
    QTRY_VERIFY_WITH_TIMEOUT(!errorSpy.isEmpty(), 2000);
    QCOMPARE(errorSpy[0].at(0).value<QLowEnergyService::ServiceError>(),
             QLowEnergyService::OperationError);
    QCOMPARE(service->error(), QLowEnergyService::OperationError);
    QCOMPARE(writeSpy.count(), 0);
    QCOMPARE(configChar.value(), QByteArray("00"));

    delete service;
}

QTEST_MAIN(tst_QLowEnergyController)

#include "tst_qlowenergycontroller.moc"
