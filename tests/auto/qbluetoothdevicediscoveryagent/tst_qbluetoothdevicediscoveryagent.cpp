/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: http://www.qt-project.org/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
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
#include <qbluetoothdevicediscoveryagent.h>
#include <qbluetoothlocaldevice.h>

QTBLUETOOTH_USE_NAMESPACE

Q_DECLARE_METATYPE(QBluetoothDeviceInfo)
Q_DECLARE_METATYPE(QBluetoothDeviceDiscoveryAgent::InquiryType)

// Maximum time to for bluetooth device scan
const int MaxScanTime = 5 * 60 * 1000;  // 5 minutes in ms
const int MaxWaitTime = 5 * 1000;  // 5 seconds in ms

class tst_QBluetoothDeviceDiscoveryAgent : public QObject
{
    Q_OBJECT

public:
    tst_QBluetoothDeviceDiscoveryAgent();
    ~tst_QBluetoothDeviceDiscoveryAgent();

public slots:
    void deviceDiscoveryDebug(const QBluetoothDeviceInfo &info);
    void finished();

private slots:
    void initTestCase();

    void tst_properties();

    void tst_startStopDeviceDiscoveries();

    void tst_deviceDiscovery_data();
    void tst_deviceDiscovery();
};

tst_QBluetoothDeviceDiscoveryAgent::tst_QBluetoothDeviceDiscoveryAgent()
{
    qRegisterMetaType<QBluetoothDeviceDiscoveryAgent::Error>("QBluetoothDeviceDiscoveryAgent::Error");
}

tst_QBluetoothDeviceDiscoveryAgent::~tst_QBluetoothDeviceDiscoveryAgent()
{
}

void tst_QBluetoothDeviceDiscoveryAgent::initTestCase()
{
    qRegisterMetaType<QBluetoothDeviceInfo>("QBluetoothDeviceInfo");
    qRegisterMetaType<QBluetoothDeviceDiscoveryAgent::InquiryType>("QBluetoothDeviceDiscoveryAgent::InquiryType");

    // turn on BT in case it is not on
    QBluetoothLocalDevice *device = new QBluetoothLocalDevice();
    if (device->hostMode() == QBluetoothLocalDevice::HostPoweredOff) {
        QSignalSpy hostModeSpy(device, SIGNAL(hostModeStateChanged(QBluetoothLocalDevice::HostMode)));
        QVERIFY(hostModeSpy.isEmpty());
        device->powerOn();
        int connectTime = 5000;  // ms
        while (hostModeSpy.count() < 1 && connectTime > 0) {
            QTest::qWait(500);
            connectTime -= 500;
        }
        QVERIFY(hostModeSpy.count() > 0);
    }
    QBluetoothLocalDevice::HostMode hostMode= device->hostMode();
    QVERIFY(hostMode == QBluetoothLocalDevice::HostConnectable
         || hostMode == QBluetoothLocalDevice::HostDiscoverable
         || hostMode == QBluetoothLocalDevice::HostDiscoverableLimitedInquiry);
    delete device;
}

void tst_QBluetoothDeviceDiscoveryAgent::tst_properties()
{
    {
        QBluetoothDeviceDiscoveryAgent discoveryAgent;

        QCOMPARE(discoveryAgent.inquiryType(), QBluetoothDeviceDiscoveryAgent::GeneralUnlimitedInquiry);
        discoveryAgent.setInquiryType(QBluetoothDeviceDiscoveryAgent::LimitedInquiry);
        QCOMPARE(discoveryAgent.inquiryType(), QBluetoothDeviceDiscoveryAgent::LimitedInquiry);
        discoveryAgent.setInquiryType(QBluetoothDeviceDiscoveryAgent::GeneralUnlimitedInquiry);
        QCOMPARE(discoveryAgent.inquiryType(), QBluetoothDeviceDiscoveryAgent::GeneralUnlimitedInquiry);
    }
}

void tst_QBluetoothDeviceDiscoveryAgent::deviceDiscoveryDebug(const QBluetoothDeviceInfo &info)
{
    qDebug() << "Discovered device:" << info.address().toString() << info.name();
}

void tst_QBluetoothDeviceDiscoveryAgent::tst_startStopDeviceDiscoveries()
{
    {
        QBluetoothDeviceDiscoveryAgent::InquiryType inquiryType = QBluetoothDeviceDiscoveryAgent::GeneralUnlimitedInquiry;
        QBluetoothDeviceDiscoveryAgent discoveryAgent;

        QVERIFY(discoveryAgent.error() == discoveryAgent.NoError);
        QVERIFY(discoveryAgent.errorString().isEmpty());
        QVERIFY(!discoveryAgent.isActive());
        QVERIFY(discoveryAgent.discoveredDevices().isEmpty());

        QSignalSpy finishedSpy(&discoveryAgent, SIGNAL(finished()));
        QSignalSpy cancelSpy(&discoveryAgent, SIGNAL(canceled()));
        QSignalSpy errorSpy(&discoveryAgent, SIGNAL(error(QBluetoothDeviceDiscoveryAgent::Error)));
        QSignalSpy discoveredSpy(&discoveryAgent, SIGNAL(deviceDiscovered(const QBluetoothDeviceInfo&)));

        // Starting case 1: start-stop, expecting cancel signal
        discoveryAgent.setInquiryType(inquiryType);
        // we should have no errors at this point.
        QVERIFY(errorSpy.isEmpty());

        discoveryAgent.start();
        QVERIFY(discoveryAgent.isActive());
        QVERIFY(errorSpy.isEmpty());
        // cancel current request.
        discoveryAgent.stop();

        // Wait for up to MaxWaitTime for the cancel to finish
        int waitTime = MaxWaitTime;
        while (cancelSpy.count() == 0 && waitTime > 0) {
            QTest::qWait(100);
            waitTime-=100;
        }

        // we should not be active anymore
        QVERIFY(!discoveryAgent.isActive());
        QVERIFY(errorSpy.isEmpty());
        QVERIFY(cancelSpy.count() == 1);
        cancelSpy.clear();
        // Starting case 2: start-start-stop, expecting cancel signal
        discoveryAgent.start();
        // we should be active now
        QVERIFY(discoveryAgent.isActive());
        QVERIFY(errorSpy.isEmpty());
        // start again. should this be error?
        discoveryAgent.start();
        QVERIFY(discoveryAgent.isActive());
        QVERIFY(errorSpy.isEmpty());
        // stop
        discoveryAgent.stop();

        // Wait for up to MaxWaitTime for the cancel to finish
        waitTime = MaxWaitTime;
        while (cancelSpy.count() == 0 && waitTime > 0) {
            QTest::qWait(100);
            waitTime-=100;
        }

        // we should not be active anymore
        QVERIFY(!discoveryAgent.isActive());
        QVERIFY(errorSpy.isEmpty());
        QVERIFY(cancelSpy.count() == 1);
        cancelSpy.clear();

        //  Starting case 3: stop
        discoveryAgent.stop();
        QVERIFY(!discoveryAgent.isActive());
        QVERIFY(errorSpy.isEmpty());

        // Expect finished signal with no error
        QVERIFY(finishedSpy.count() == 0);
        QVERIFY(discoveryAgent.error() == discoveryAgent.NoError);
        QVERIFY(discoveryAgent.errorString().isEmpty());


        // Starting case 4: start-stop-start-stop, expecting only 1 cancel signal
        discoveryAgent.start();
        QVERIFY(discoveryAgent.isActive());
        QVERIFY(errorSpy.isEmpty());
        // cancel current request.
        discoveryAgent.stop();
        // start a new one
        discoveryAgent.start();
        // we should be active now
        QVERIFY(discoveryAgent.isActive());
        QVERIFY(errorSpy.isEmpty());
        // stop
        discoveryAgent.stop();

        // Wait for up to MaxWaitTime for the cancel to finish
        waitTime = MaxWaitTime;
        while (waitTime > 0) {
            QTest::qWait(100);
            waitTime-=100;
        }

        // we should not be active anymore
        QVERIFY(!discoveryAgent.isActive());
        QVERIFY(errorSpy.isEmpty());
        // should only have 1 cancel
        QVERIFY(cancelSpy.count() == 1);
        cancelSpy.clear();

        // Starting case 5: start-stop-start: expecting finished signal & no cancel
        discoveryAgent.start();
        QVERIFY(discoveryAgent.isActive());
        QVERIFY(errorSpy.isEmpty());
        // cancel current request.
        discoveryAgent.stop();
        // start a new one
        discoveryAgent.start();
        // we should be active now
        QVERIFY(discoveryAgent.isActive());
        QVERIFY(errorSpy.isEmpty());

        // Wait for up to MaxScanTime for the cancel to finish
        waitTime = MaxScanTime;
        while (finishedSpy.count() == 0 && waitTime > 0) {
            QTest::qWait(1000);
            waitTime-=1000;
        }

        // we should not be active anymore
        QVERIFY(!discoveryAgent.isActive());
        QVERIFY(errorSpy.isEmpty());
        // should only have 1 cancel
        QVERIFY(finishedSpy.count() == 1);
        QVERIFY(cancelSpy.isEmpty());
    }
}

void tst_QBluetoothDeviceDiscoveryAgent::finished()
{
    qDebug() << "Finished called";
}

void tst_QBluetoothDeviceDiscoveryAgent::tst_deviceDiscovery_data()
{
    QTest::addColumn<QBluetoothDeviceDiscoveryAgent::InquiryType>("inquiryType");

    QTest::newRow("general unlimited inquiry") << QBluetoothDeviceDiscoveryAgent::GeneralUnlimitedInquiry;
    QTest::newRow("limited inquiry") << QBluetoothDeviceDiscoveryAgent::LimitedInquiry;
}

void tst_QBluetoothDeviceDiscoveryAgent::tst_deviceDiscovery()
{
    {
        QFETCH(QBluetoothDeviceDiscoveryAgent::InquiryType, inquiryType);

        QBluetoothDeviceDiscoveryAgent discoveryAgent;
        QVERIFY(discoveryAgent.error() == discoveryAgent.NoError);
        QVERIFY(discoveryAgent.errorString().isEmpty());
        QVERIFY(!discoveryAgent.isActive());

        QVERIFY(discoveryAgent.discoveredDevices().isEmpty());

        QSignalSpy finishedSpy(&discoveryAgent, SIGNAL(finished()));
        QSignalSpy errorSpy(&discoveryAgent, SIGNAL(error(QBluetoothDeviceDiscoveryAgent::Error)));
        QSignalSpy discoveredSpy(&discoveryAgent, SIGNAL(deviceDiscovered(const QBluetoothDeviceInfo&)));
//        connect(&discoveryAgent, SIGNAL(finished()), this, SLOT(finished()));
//        connect(&discoveryAgent, SIGNAL(deviceDiscovered(const QBluetoothDeviceInfo&)),
//                this, SLOT(deviceDiscoveryDebug(const QBluetoothDeviceInfo&)));

        discoveryAgent.setInquiryType(inquiryType);
        discoveryAgent.start();

        QVERIFY(discoveryAgent.isActive());

        // Wait for up to MaxScanTime for the scan to finish
        int scanTime = MaxScanTime;
        while (finishedSpy.count() == 0 && scanTime > 0) {
            QTest::qWait(15000);
            scanTime -= 15000;
        }
        qDebug() << scanTime << MaxScanTime;
        // verify that we are finished
        QVERIFY(!discoveryAgent.isActive());
        // stop
        discoveryAgent.stop();
        QVERIFY(!discoveryAgent.isActive());
        qDebug() << "Scan time left:" << scanTime;
        // Expect finished signal with no error
        QVERIFY(finishedSpy.count() == 1);
        QVERIFY(errorSpy.isEmpty());
        QVERIFY(discoveryAgent.error() == discoveryAgent.NoError);
        QVERIFY(discoveryAgent.errorString().isEmpty());

        // verify that the list is as big as the signals received.
        QVERIFY(discoveredSpy.count() == discoveryAgent.discoveredDevices().length());
        // verify that there really was some devices in the array
        QVERIFY(discoveredSpy.count() > 0);

        // All returned QBluetoothDeviceInfo should be valid.
        while (!discoveredSpy.isEmpty()) {
            const QBluetoothDeviceInfo info =
                qvariant_cast<QBluetoothDeviceInfo>(discoveredSpy.takeFirst().at(0));
            QVERIFY(info.isValid());
        }
    }
}

QTEST_MAIN(tst_QBluetoothDeviceDiscoveryAgent)

#include "tst_qbluetoothdevicediscoveryagent.moc"
