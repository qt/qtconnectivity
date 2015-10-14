/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
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
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtBluetooth/qbluetoothaddress.h>
#include <QtBluetooth/qbluetoothdevicediscoveryagent.h>
#include <QtBluetooth/qbluetoothdeviceinfo.h>
#include <QtBluetooth/qlowenergyadvertisingdata.h>
#include <QtBluetooth/qlowenergyadvertisingparameters.h>
#include <QtBluetooth/qlowenergycontroller.h>
#include <QtCore/qscopedpointer.h>
#include <QtTest/qsignalspy.h>
#include <QtTest/QtTest>

#include <algorithm>

class TestQLowEnergyControllerGattServer : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void advertisingParameters();
    void advertisingData();
    void advertisedData();
    void controllerType();

private:
    QBluetoothAddress m_serverAddress;
    QBluetoothDeviceInfo m_serverInfo;
};


void TestQLowEnergyControllerGattServer::initTestCase()
{
    const QString serverAddress = qgetenv("QT_BT_GATTSERVER_TEST_ADDRESS");
    if (serverAddress.isEmpty())
        return;
    m_serverAddress = QBluetoothAddress(serverAddress);
    QVERIFY(!m_serverAddress.isNull());
}

void TestQLowEnergyControllerGattServer::advertisingParameters()
{
    QLowEnergyAdvertisingParameters params;
    QCOMPARE(params, QLowEnergyAdvertisingParameters());
    QCOMPARE(params.filterPolicy(), QLowEnergyAdvertisingParameters::IgnoreWhiteList);
    QCOMPARE(params.minimumInterval(), 1280);
    QCOMPARE(params.maximumInterval(), 1280);
    QCOMPARE(params.mode(), QLowEnergyAdvertisingParameters::AdvInd);
    QVERIFY(params.whiteList().isEmpty());

    params.setInterval(100, 200);
    QCOMPARE(params.minimumInterval(), 100);
    QCOMPARE(params.maximumInterval(), 200);
    params.setInterval(200, 100);
    QCOMPARE(params.minimumInterval(), 200);
    QCOMPARE(params.maximumInterval(), 200);

    params.setMode(QLowEnergyAdvertisingParameters::AdvScanInd);
    QCOMPARE(params.mode(), QLowEnergyAdvertisingParameters::AdvScanInd);

    const auto whiteList = QList<QLowEnergyAdvertisingParameters::AddressInfo>()
            << QLowEnergyAdvertisingParameters::AddressInfo(QBluetoothAddress(),
                                                            QLowEnergyController::PublicAddress);
    params.setWhiteList(whiteList, QLowEnergyAdvertisingParameters::UseWhiteListForConnecting);
    QCOMPARE(params.whiteList(), whiteList);
    QCOMPARE(params.filterPolicy(), QLowEnergyAdvertisingParameters::UseWhiteListForConnecting);
    QVERIFY(params != QLowEnergyAdvertisingParameters());
}

void TestQLowEnergyControllerGattServer::advertisingData()
{
    QLowEnergyAdvertisingData data;
    QCOMPARE(data, QLowEnergyAdvertisingData());
    QCOMPARE(data.discoverability(), QLowEnergyAdvertisingData::DiscoverabilityNone);
    QCOMPARE(data.includePowerLevel(), false);
    QCOMPARE(data.localName(), QString());
    QCOMPARE(data.manufacturerData(), QByteArray());
    QCOMPARE(data.manufacturerId(), QLowEnergyAdvertisingData::invalidManufacturerId());
    QVERIFY(data.services().isEmpty());

    data.setDiscoverability(QLowEnergyAdvertisingData::DiscoverabilityLimited);
    QCOMPARE(data.discoverability(), QLowEnergyAdvertisingData::DiscoverabilityLimited);

    data.setIncludePowerLevel(true);
    QCOMPARE(data.includePowerLevel(), true);

    data.setLocalName("A device name");
    QCOMPARE(data.localName(), QString("A device name"));

    data.setManufacturerData(0xfffd, "some data");
    QCOMPARE(data.manufacturerId(), quint16(0xfffd));
    QCOMPARE(data.manufacturerData(), QByteArray("some data"));

    const auto services = QList<QBluetoothUuid>() << QBluetoothUuid::CurrentTimeService
                                                  << QBluetoothUuid::DeviceInformation;
    data.setServices(services);
    QCOMPARE(data.services(), services);

    QByteArray rawData(7, 'x');
    data.setRawData(rawData);
    QCOMPARE(data.rawData(), rawData);

    QVERIFY(data != QLowEnergyAdvertisingData());
}

void TestQLowEnergyControllerGattServer::advertisedData()
{
    if (m_serverAddress.isNull())
        QSKIP("No server address provided");
    QBluetoothDeviceDiscoveryAgent discoveryAgent;
    discoveryAgent.start();
    QSignalSpy spy(&discoveryAgent, SIGNAL(finished()));
    QVERIFY(spy.wait(30000));
    const QList<QBluetoothDeviceInfo> devices = discoveryAgent.discoveredDevices();
    const auto it = std::find_if(devices.constBegin(), devices.constEnd(),
        [this](const QBluetoothDeviceInfo &device) { return device.address() == m_serverAddress; });
    QVERIFY(it != devices.constEnd());
    m_serverInfo = *it;

    // BlueZ seems to interfere with the advertising in some way, so that in addition to the name
    // we set, the host name of the machine is also sent. Therefore we cannot guarantee that "our"
    // name is seen on the scanning machine.
    // QCOMPARE(m_serverInfo.name(), QString("Qt GATT server"));

    QCOMPARE(m_serverInfo.serviceUuids(),
             QList<QBluetoothUuid>() << QBluetoothUuid::AlertNotificationService);
}

void TestQLowEnergyControllerGattServer::controllerType()
{
    const QScopedPointer<QLowEnergyController> controller(QLowEnergyController::createPeripheral());
    QVERIFY(!controller.isNull());
    QCOMPARE(controller->role(), QLowEnergyController::PeripheralRole);
}

QTEST_MAIN(TestQLowEnergyControllerGattServer)

#include "tst_qlowenergycontroller-gattserver.moc"
