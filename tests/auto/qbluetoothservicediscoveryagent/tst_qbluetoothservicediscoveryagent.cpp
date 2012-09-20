/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
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

#include <QDebug>
#include <QVariant>
#include <QStringList>

#include <qbluetoothaddress.h>
#include <qbluetoothdevicediscoveryagent.h>
#include <qbluetoothservicediscoveryagent.h>
#include <qbluetoothlocaldevice.h>

QTBLUETOOTH_USE_NAMESPACE

Q_DECLARE_METATYPE(QBluetoothDeviceInfo)
Q_DECLARE_METATYPE(QBluetoothServiceDiscoveryAgent::Error)

// Maximum time to for bluetooth device scan
const int MaxScanTime = 5 * 60 * 1000;  // 5 minutes in ms

class tst_QBluetoothServiceDiscoveryAgent : public QObject
{
    Q_OBJECT

public:
    tst_QBluetoothServiceDiscoveryAgent();
    ~tst_QBluetoothServiceDiscoveryAgent();

public slots:
    void deviceDiscoveryDebug(const QBluetoothDeviceInfo &info);
    void serviceDiscoveryDebug(const QBluetoothServiceInfo &info);
    void serviceError(const QBluetoothServiceDiscoveryAgent::Error err);

private slots:
    void initTestCase();

    void tst_serviceDiscovery_data();
    void tst_serviceDiscovery();

private:
    QList<QBluetoothDeviceInfo> devices;
};

tst_QBluetoothServiceDiscoveryAgent::tst_QBluetoothServiceDiscoveryAgent()
{
    // start Bluetooth if not started
    QBluetoothLocalDevice *device = new QBluetoothLocalDevice();
    device->powerOn();
    delete device;
    // wait for the device to switch bluetooth mode.
    QTest::qWait(1000);
    qRegisterMetaType<QBluetoothDeviceInfo>("QBluetoothDeviceInfo");
    qRegisterMetaType<QBluetoothServiceInfo>("QBluetoothServiceInfo");
    qRegisterMetaType<QList<QBluetoothUuid> >("QList<QBluetoothUuid>");
    qRegisterMetaType<QBluetoothServiceDiscoveryAgent::Error>("QBluetoothServiceDiscoveryAgent::Error");
    qRegisterMetaType<QBluetoothDeviceDiscoveryAgent::Error>("QBluetoothDeviceDiscoveryAgent::Error");

}

tst_QBluetoothServiceDiscoveryAgent::~tst_QBluetoothServiceDiscoveryAgent()
{
}

void tst_QBluetoothServiceDiscoveryAgent::deviceDiscoveryDebug(const QBluetoothDeviceInfo &info)
{
    qDebug() << "Discovered device:" << info.address().toString() << info.name();
}


void tst_QBluetoothServiceDiscoveryAgent::serviceError(const QBluetoothServiceDiscoveryAgent::Error err)
{
    qDebug() << "Service discovery error" << err;
}

void tst_QBluetoothServiceDiscoveryAgent::initTestCase()
{
#if 1
    QBluetoothDeviceDiscoveryAgent discoveryAgent;

    QSignalSpy finishedSpy(&discoveryAgent, SIGNAL(finished()));
    QSignalSpy errorSpy(&discoveryAgent, SIGNAL(error(QBluetoothDeviceDiscoveryAgent::Error)));
    QSignalSpy discoveredSpy(&discoveryAgent, SIGNAL(deviceDiscovered(const QBluetoothDeviceInfo&)));
//    connect(&discoveryAgent, SIGNAL(deviceDiscovered(const QBluetoothDeviceInfo&)),
//            this, SLOT(deviceDiscoveryDebug(const QBluetoothDeviceInfo&)));

    discoveryAgent.start();

    // Wait for up to MaxScanTime for the scan to finish
    int scanTime = MaxScanTime;
    while (finishedSpy.count() == 0 && scanTime > 0) {
        QTest::qWait(1000);
        scanTime -= 1000;
    }
//    qDebug() << "Scan time left:" << scanTime;

    // Expect finished signal with no error
    QVERIFY(finishedSpy.count() == 1);
    QVERIFY(errorSpy.isEmpty());

    devices = discoveryAgent.discoveredDevices();
#else
    devices.append(QBluetoothDeviceInfo(QBluetoothAddress(Q_UINT64_C(0x001e3a81ba69)), "Yuna", 0));
#endif
}

void tst_QBluetoothServiceDiscoveryAgent::serviceDiscoveryDebug(const QBluetoothServiceInfo &info)
{
    qDebug() << "Discovered service on"
             << info.device().name() << info.device().address().toString();
    qDebug() << "\tService name:" << info.serviceName() << "cached" << info.device().isCached();
    qDebug() << "\tDescription:"
             << info.attribute(QBluetoothServiceInfo::ServiceDescription).toString();
    qDebug() << "\tProvider:" << info.attribute(QBluetoothServiceInfo::ServiceProvider).toString();
    qDebug() << "\tL2CAP protocol service multiplexer:" << info.protocolServiceMultiplexer();
    qDebug() << "\tRFCOMM server channel:" << info.serverChannel();
}

static void dumpAttributeVariant(const QVariant &var, const QString indent)
{
    if (!var.isValid()) {
        qDebug("%sEmpty", indent.toLocal8Bit().constData());
        return;
    }

    if (var.userType() == qMetaTypeId<QBluetoothServiceInfo::Sequence>()) {
        qDebug("%sSequence", indent.toLocal8Bit().constData());
        const QBluetoothServiceInfo::Sequence *sequence = static_cast<const QBluetoothServiceInfo::Sequence *>(var.data());
        foreach (const QVariant &v, *sequence)
            dumpAttributeVariant(v, indent + '\t');
    } else if (var.userType() == qMetaTypeId<QBluetoothServiceInfo::Alternative>()) {
        qDebug("%sAlternative", indent.toLocal8Bit().constData());
        const QBluetoothServiceInfo::Alternative *alternative = static_cast<const QBluetoothServiceInfo::Alternative *>(var.data());
        foreach (const QVariant &v, *alternative)
            dumpAttributeVariant(v, indent + '\t');
    } else if (var.userType() == qMetaTypeId<QBluetoothUuid>()) {
        QBluetoothUuid uuid = var.value<QBluetoothUuid>();
        switch (uuid.minimumSize()) {
        case 0:
            qDebug("%suuid NULL", indent.toLocal8Bit().constData());
            break;
        case 2:
            qDebug("%suuid %04x", indent.toLocal8Bit().constData(), uuid.toUInt16());
            break;
        case 4:
            qDebug("%suuid %08x", indent.toLocal8Bit().constData(), uuid.toUInt32());
            break;
        case 16: {
            qDebug("%suuid %s", indent.toLocal8Bit().constData(), QByteArray(reinterpret_cast<const char *>(uuid.toUInt128().data), 16).toHex().constData());
            break;
        }
        default:
            qDebug("%suuid ???", indent.toLocal8Bit().constData());
        }
    } else {
        switch (var.userType()) {
        case QVariant::UInt:
            qDebug("%suint %u", indent.toLocal8Bit().constData(), var.toUInt());
            break;
        case QVariant::Int:
            qDebug("%sint %d", indent.toLocal8Bit().constData(), var.toInt());
            break;
        case QVariant::String:
            qDebug("%sstring %s", indent.toLocal8Bit().constData(), var.toString().toLocal8Bit().constData());
            break;
        case QVariant::Bool:
            qDebug("%sbool %d", indent.toLocal8Bit().constData(), var.toBool());
            break;
        case QVariant::Url:
            qDebug("%surl %s", indent.toLocal8Bit().constData(), var.toUrl().toString().toLocal8Bit().constData());
            break;
        default:
            qDebug("%sunknown", indent.toLocal8Bit().constData());
        }
    }
}

static inline void dumpServiceInfoAttributes(const QBluetoothServiceInfo &info)
{
    foreach (quint16 id, info.attributes()) {
        dumpAttributeVariant(info.attribute(id), QString("\t"));
    }
}


void tst_QBluetoothServiceDiscoveryAgent::tst_serviceDiscovery_data()
{
    if (devices.isEmpty())
        QSKIP("This test requires an in-range bluetooth device");

    QTest::addColumn<QBluetoothDeviceInfo>("deviceInfo");
    QTest::addColumn<QList<QBluetoothUuid> >("uuidFilter");
    QTest::addColumn<QBluetoothServiceDiscoveryAgent::Error>("serviceDiscoveryError");

    // Only need to test the first 5 live devices
    int max = 5;
    foreach (const QBluetoothDeviceInfo &info, devices) {
        if (info.isCached())
            continue;
        QTest::newRow("default filter") << info << QList<QBluetoothUuid>()
            << QBluetoothServiceDiscoveryAgent::NoError;
        if (!--max)
            break;
        //QTest::newRow("public browse group") << info << (QList<QBluetoothUuid>() << QBluetoothUuid::PublicBrowseGroup);
        //QTest::newRow("l2cap") << info << (QList<QBluetoothUuid>() << QBluetoothUuid::L2cap);
    }
    QTest::newRow("all devices") << QBluetoothDeviceInfo() << QList<QBluetoothUuid>()
        << QBluetoothServiceDiscoveryAgent::NoError;
}


void tst_QBluetoothServiceDiscoveryAgent::tst_serviceDiscovery()
{
    // Not all devices respond to SDP, so allow for a failure
    static int expected_failures = 0;

    if (devices.isEmpty())
        QSKIP("This test requires an in-range bluetooth device");

    QFETCH(QBluetoothDeviceInfo, deviceInfo);
    QFETCH(QList<QBluetoothUuid>, uuidFilter);
    QFETCH(QBluetoothServiceDiscoveryAgent::Error, serviceDiscoveryError);

    qDebug() << "Doing address" << deviceInfo.address().toString();
    QBluetoothServiceDiscoveryAgent discoveryAgent(deviceInfo.address());

    QVERIFY(!discoveryAgent.isActive());

    QVERIFY(discoveryAgent.discoveredServices().isEmpty());

    QVERIFY(discoveryAgent.uuidFilter().isEmpty());

    discoveryAgent.setUuidFilter(uuidFilter);

    QVERIFY(discoveryAgent.uuidFilter() == uuidFilter);

    QSignalSpy finishedSpy(&discoveryAgent, SIGNAL(finished()));
    QSignalSpy errorSpy(&discoveryAgent, SIGNAL(error(QBluetoothServiceDiscoveryAgent::Error)));
    QSignalSpy discoveredSpy(&discoveryAgent, SIGNAL(serviceDiscovered(QBluetoothServiceInfo)));
//    connect(&discoveryAgent, SIGNAL(serviceDiscovered(QBluetoothServiceInfo)),
//            this, SLOT(serviceDiscoveryDebug(QBluetoothServiceInfo)));
    connect(&discoveryAgent, SIGNAL(error(QBluetoothServiceDiscoveryAgent::Error)),
            this, SLOT(serviceError(QBluetoothServiceDiscoveryAgent::Error)));

    discoveryAgent.start();

    QVERIFY(discoveryAgent.isActive());

    // Wait for up to MaxScanTime for the scan to finish
    int scanTime = MaxScanTime;
    while (finishedSpy.count() == 0 && scanTime > 0) {
        QTest::qWait(1000);
        scanTime -= 1000;
    }

    if (discoveryAgent.error() && expected_failures++ < 2){
        qDebug() << "Device failed to respond to SDP, skipping device" << discoveryAgent.error() << discoveryAgent.errorString();
        return;
    }

    QVERIFY(discoveryAgent.error() == serviceDiscoveryError);
    QVERIFY(discoveryAgent.errorString() == QString());

    // Expect finished signal with no error
    QVERIFY(finishedSpy.count() == 1);
    QVERIFY(errorSpy.isEmpty());

    //if (discoveryAgent.discoveredServices().count() && expected_failures++ <2){
    if (discoveredSpy.isEmpty() && expected_failures++ < 2){
        qDebug() << "Device failed to return any results, skipping device" << discoveryAgent.discoveredServices().count();
        return;
    }

    // All returned QBluetoothServiceInfo should be valid.
    while (!discoveredSpy.isEmpty()) {
        const QVariant v = discoveredSpy.takeFirst().at(0);

        // Work around limitation in QMetaType and moc.
        // QBluetoothServiceInfo is registered with metatype as QBluetoothServiceInfo
        // moc sees it as the unqualified QBluetoothServiceInfo.
        if (qstrcmp(v.typeName(), "QBluetoothServiceInfo") == 0) {
            const QBluetoothServiceInfo info =
                *reinterpret_cast<const QBluetoothServiceInfo*>(v.constData());

            QVERIFY(info.isValid());

#if 0
            qDebug() << info.device().name() << info.device().address().toString();
            qDebug() << "\tService name:" << info.serviceName();
            if (info.protocolServiceMultiplexer() >= 0)
                qDebug() << "\tL2CAP protocol service multiplexer:" << info.protocolServiceMultiplexer();
            if (info.serverChannel() >= 0)
                qDebug() << "\tRFCOMM server channel:" << info.serverChannel();
            //dumpServiceInfoAttributes(info);
#endif
        }
    }

    QVERIFY(discoveryAgent.discoveredServices().count() != 0);
    discoveryAgent.clear();
    QVERIFY(discoveryAgent.discoveredServices().count() == 0);

    discoveryAgent.stop();
    QVERIFY(!discoveryAgent.isActive());
}

QTEST_MAIN(tst_QBluetoothServiceDiscoveryAgent)

#include "tst_qbluetoothservicediscoveryagent.moc"
