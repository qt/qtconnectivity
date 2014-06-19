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
#include <QLowEnergyController>
#include <QLowEnergyControllerNew>
#include <QLowEnergyServiceInfo>

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
    void serviceConnected(const QLowEnergyServiceInfo &info);
    void serviceDisconnected(const QLowEnergyServiceInfo &info);

private slots:
    void initTestCase();
    void cleanupTestCase();
    void tst_verifyAllServices();
    void tst_connect();
    void tst_connectNew();
    void tst_defaultBehavior();

private:
    void verifyServiceProperties(const QLowEnergyServiceInfo &info);

    QBluetoothServiceDiscoveryAgent *agent;
    QBluetoothAddress remoteDevice;
    QList<QLowEnergyServiceInfo> foundServices;
};


tst_QLowEnergyController::tst_QLowEnergyController()
{
    qRegisterMetaType<QLowEnergyServiceInfo>("QLowEnergyServiceInfo");
    qRegisterMetaType<QLowEnergyController::Error>("QLowEnergyController::Error");
    qRegisterMetaType<QLowEnergyCharacteristicInfo>("QLowEnergyCharacteristicInfo");

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

void tst_QLowEnergyController::serviceConnected(const QLowEnergyServiceInfo &/*info*/)
{

}

void tst_QLowEnergyController::serviceDisconnected(const QLowEnergyServiceInfo &/*info*/)
{

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
        QCOMPARE(chars[1].value(), QByteArray("0000"));
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
        QCOMPARE(chars[0].descriptors().at(0).value(), QByteArray("0000"));
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
        QCOMPARE(chars[0].descriptors().at(0).value(), QByteArray("0000"));
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
        QCOMPARE(chars[0].descriptors().at(0).value(), QByteArray("0000"));
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
        QCOMPARE(chars[0].descriptors().at(0).value(), QByteArray("0000"));
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
            QCOMPARE(chars[0].descriptors().at(0).value(), QByteArray("0000"));
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
            QCOMPARE(chars[0].descriptors().at(0).value(), QByteArray("0000"));
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
            QCOMPARE(chars[0].descriptors().at(0).value(), QByteArray("0000"));
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
    if (localAdapters.isEmpty())
        QSKIP("No local Bluetooth device found. Skipping test.");

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

        // initiate characteristic discovery
        foreach (QLowEnergyService *service, savedReferences) {
            //qDebug() << "Discoverying" << service->serviceUuid();
            QSignalSpy stateSpy(service,
                                SIGNAL(stateChanged(QLowEnergyService::ServiceState)));
            QSignalSpy errorSpy(service, SIGNAL(error(QLowEnergyService::ServiceError)));
            service->discoverDetails();

            QTRY_VERIFY_WITH_TIMEOUT(
                        service->state() == QLowEnergyService::ServiceDiscovered, 10000);

            QCOMPARE(errorSpy.count(), 0); //no error
            QCOMPARE(stateSpy.count(), 2); //

//            for (int i = 0; i < stateSpy.count(); i++) {
//                const QVariant v = stateSpy[i].at(0);
//                //qDebug() << v;
//            }
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
        }
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
    }

    // Test explicit local adapter
    if (!foundAddresses.isEmpty()) {
        QLowEnergyControllerNew controlExplicitAdapter(randomAddress,
                                                       foundAddresses[0]);
        QCOMPARE(controlExplicitAdapter.remoteAddress(), randomAddress);
        QCOMPARE(controlExplicitAdapter.localAddress(), foundAddresses[0]);
        QCOMPARE(controlExplicitAdapter.state(),
                 QLowEnergyControllerNew::UnconnectedState);
    }
}

QTEST_MAIN(tst_QLowEnergyController)

#include "tst_qlowenergycontroller.moc"
