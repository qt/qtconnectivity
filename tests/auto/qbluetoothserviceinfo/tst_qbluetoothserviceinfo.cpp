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
#include <QUuid>

#include <QDebug>

#include <qbluetoothdeviceinfo.h>
#include <qbluetoothserviceinfo.h>
#include <qbluetoothaddress.h>
#include <qbluetoothlocaldevice.h>
#include <qbluetoothuuid.h>

QTCONNECTIVITY_USE_NAMESPACE

Q_DECLARE_METATYPE(QBluetoothUuid::ProtocolUuid)
Q_DECLARE_METATYPE(QUuid)
Q_DECLARE_METATYPE(QBluetoothServiceInfo::Protocol)

class tst_QBluetoothServiceInfo : public QObject
{
    Q_OBJECT

public:
    tst_QBluetoothServiceInfo();
    ~tst_QBluetoothServiceInfo();

private slots:
    void initTestCase();

    void tst_construction();

    void tst_assignment_data();
    void tst_assignment();
};

tst_QBluetoothServiceInfo::tst_QBluetoothServiceInfo()
{
}

tst_QBluetoothServiceInfo::~tst_QBluetoothServiceInfo()
{
}

void tst_QBluetoothServiceInfo::initTestCase()
{
    qRegisterMetaType<QBluetoothUuid::ProtocolUuid>("QBluetoothUuid::ProtocolUuid");
    qRegisterMetaType<QUuid>("QUuid");
    qRegisterMetaType<QBluetoothServiceInfo::Protocol>("QBluetoothServiceInfo::Protocol");
    // start Bluetooth if not started
    QBluetoothLocalDevice *device = new QBluetoothLocalDevice();
    device->powerOn();
    delete device;
}

void tst_QBluetoothServiceInfo::tst_construction()
{
    const QString serviceName("My Service");
    const QBluetoothDeviceInfo deviceInfo(QBluetoothAddress("001122334455"), "Test Device", 0);

    {
        QBluetoothServiceInfo serviceInfo;

        QVERIFY(!serviceInfo.isValid());
        QVERIFY(!serviceInfo.isComplete());
    }

    {
        QBluetoothServiceInfo serviceInfo;
        serviceInfo.setServiceName(serviceName);
        serviceInfo.setDevice(deviceInfo);

        QVERIFY(serviceInfo.isValid());

        QCOMPARE(serviceInfo.serviceName(), serviceName);
        QCOMPARE(serviceInfo.device().address(), deviceInfo.address());

        QBluetoothServiceInfo copyInfo(serviceInfo);

        QVERIFY(copyInfo.isValid());

        QCOMPARE(copyInfo.serviceName(), serviceName);
        QCOMPARE(copyInfo.device().address(), deviceInfo.address());
    }
}

void tst_QBluetoothServiceInfo::tst_assignment_data()
{
    QTest::addColumn<QUuid>("uuid");
    QTest::addColumn<QBluetoothUuid::ProtocolUuid>("protocolUuid");
    QTest::addColumn<QBluetoothServiceInfo::Protocol>("serviceInfoProtocol");

    QTest::newRow("assignment_data")
        << QUuid(0x67c8770b, 0x44f1, 0x410a, 0xab, 0x9a, 0xf9, 0xb5, 0x44, 0x6f, 0x13, 0xee)
        << QBluetoothUuid::L2cap << QBluetoothServiceInfo::L2capProtocol;
}

void tst_QBluetoothServiceInfo::tst_assignment()
{
    QFETCH(QUuid, uuid);
    QFETCH(QBluetoothUuid::ProtocolUuid, protocolUuid);
    QFETCH(QBluetoothServiceInfo::Protocol, serviceInfoProtocol);

    const QString serviceName("My Service");
    const QBluetoothDeviceInfo deviceInfo(QBluetoothAddress("001122334455"), "Test Device", 0);

    QBluetoothServiceInfo serviceInfo;
    serviceInfo.setServiceName(serviceName);
    serviceInfo.setDevice(deviceInfo);

    QVERIFY(serviceInfo.isValid());

    {
        QBluetoothServiceInfo copyInfo = serviceInfo;

        QVERIFY(copyInfo.isValid());

        QCOMPARE(copyInfo.serviceName(), serviceName);
        QCOMPARE(copyInfo.device().address(), deviceInfo.address());
    }

    {
        QBluetoothServiceInfo copyInfo;

        QVERIFY(!copyInfo.isValid());

        copyInfo = serviceInfo;

        QVERIFY(copyInfo.isValid());

        QCOMPARE(copyInfo.serviceName(), serviceName);
        QCOMPARE(copyInfo.device().address(), deviceInfo.address());
    }

    {
        QBluetoothServiceInfo copyInfo1;
        QBluetoothServiceInfo copyInfo2;

        QVERIFY(!copyInfo1.isValid());
        QVERIFY(!copyInfo2.isValid());

        copyInfo1 = copyInfo2 = serviceInfo;

        QVERIFY(copyInfo1.isValid());
        QVERIFY(copyInfo2.isValid());

        QCOMPARE(copyInfo1.serviceName(), serviceName);
        QCOMPARE(copyInfo2.serviceName(), serviceName);
        QCOMPARE(copyInfo1.device().address(), deviceInfo.address());
        QCOMPARE(copyInfo2.device().address(), deviceInfo.address());
    }

    {
        QBluetoothServiceInfo copyInfo;
        QVERIFY(!copyInfo.isValid());
        copyInfo = serviceInfo;

        copyInfo.setAttribute(QBluetoothServiceInfo::ProtocolDescriptorList, uuid);
        QVERIFY(copyInfo.contains(QBluetoothServiceInfo::ProtocolDescriptorList));
        QVERIFY(copyInfo.isComplete());
        QVERIFY(copyInfo.attributes().count() > 0);

        copyInfo.removeAttribute(QBluetoothServiceInfo::ProtocolDescriptorList);
        QVERIFY(!copyInfo.contains(QBluetoothServiceInfo::ProtocolDescriptorList));
        QVERIFY(!copyInfo.isComplete());
    }

    {
        QBluetoothServiceInfo copyInfo;
        QVERIFY(!copyInfo.isValid());
        copyInfo = serviceInfo;

        QVERIFY(copyInfo.serverChannel() == -1);
        QVERIFY(copyInfo.protocolServiceMultiplexer() == -1);

        QBluetoothServiceInfo::Sequence protocolDescriptorList;
        QBluetoothServiceInfo::Sequence protocol;
        protocol << QVariant::fromValue(QBluetoothUuid(protocolUuid));
        protocolDescriptorList.append(QVariant::fromValue(protocol));
        protocol.clear();

        protocolDescriptorList.append(QVariant::fromValue(protocol));
        copyInfo.setAttribute(QBluetoothServiceInfo::ProtocolDescriptorList,
                                 protocolDescriptorList);
        QVERIFY(copyInfo.serverChannel() == -1);
        QVERIFY(copyInfo.protocolServiceMultiplexer() != -1);
        QVERIFY(copyInfo.socketProtocol() == serviceInfoProtocol);
    }

    {
        QBluetoothServiceInfo copyInfo;
        QVERIFY(!copyInfo.isValid());
        copyInfo = serviceInfo;
        QVERIFY(!copyInfo.isRegistered());

        QVERIFY(copyInfo.registerService());
        QVERIFY(copyInfo.isRegistered());

        QVERIFY(copyInfo.unregisterService());
        QVERIFY(!copyInfo.isRegistered());
    }
}

QTEST_MAIN(tst_QBluetoothServiceInfo)

#include "tst_qbluetoothserviceinfo.moc"
