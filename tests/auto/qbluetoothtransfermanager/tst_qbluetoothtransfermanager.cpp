/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
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
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtTest/QtTest>

#include <QDebug>
#include <QMap>
#include <QTextStream>

#include <qbluetoothtransferrequest.h>
#include <qbluetoothtransfermanager.h>
#include <qbluetoothtransferreply.h>
#include <qbluetoothaddress.h>
#include <qbluetoothlocaldevice.h>

#include <qbluetoothdeviceinfo.h>
#include <qbluetoothserviceinfo.h>
#include <qbluetoothservicediscoveryagent.h>
#include <btclient.h>

QTBLUETOOTH_USE_NAMESPACE

typedef QMap<int,QVariant> tst_QBluetoothTransferManager_QParameterMap;
Q_DECLARE_METATYPE(tst_QBluetoothTransferManager_QParameterMap)

char BTADDRESS[] = "00:00:00:00:00:00";

static const int MaxConnectTime = 60 * 1000;   // 1 minute in ms

class tst_QBluetoothTransferManager : public QObject
{
    Q_OBJECT

public:
    tst_QBluetoothTransferManager();
    ~tst_QBluetoothTransferManager();

private slots:
    void initTestCase();

    void tst_construction();

    void tst_put_data();
    void tst_put();

    void tst_putAbort_data();
    void tst_putAbort();

    void tst_attribute_data();
    void tst_attribute();

    void tst_operation_data();
    void tst_operation();

    void tst_manager_data();
    void tst_manager();

public slots:
    void serviceDiscovered(const QBluetoothServiceInfo &info);
    void finished();
    void error(QBluetoothServiceDiscoveryAgent::Error error);
private:
    bool done_discovery;
};

tst_QBluetoothTransferManager::tst_QBluetoothTransferManager()
{
}

tst_QBluetoothTransferManager::~tst_QBluetoothTransferManager()
{
}

void tst_QBluetoothTransferManager::initTestCase()
{
    // start Bluetooth if not started
    QBluetoothLocalDevice *device = new QBluetoothLocalDevice();
    device->powerOn();
    delete device;

    // Go find an echo server for BTADDRESS
    QBluetoothServiceDiscoveryAgent *sda = new QBluetoothServiceDiscoveryAgent(this);
    connect(sda, SIGNAL(serviceDiscovered(QBluetoothServiceInfo)), this, SLOT(serviceDiscovered(QBluetoothServiceInfo)));
    connect(sda, SIGNAL(error(QBluetoothServiceDiscoveryAgent::Error)), this, SLOT(error(QBluetoothServiceDiscoveryAgent::Error)));
    connect(sda, SIGNAL(finished()), this, SLOT(finished()));

    qDebug() << "Starting discovery";
    done_discovery = false;
    memset(BTADDRESS, 0, 18);

    sda->setUuidFilter(QBluetoothUuid(QString(ECHO_SERVICE_UUID)));
    sda->start(QBluetoothServiceDiscoveryAgent::MinimalDiscovery);

    for (int connectTime = MaxConnectTime; !done_discovery && connectTime > 0; connectTime -= 1000)
        QTest::qWait(1000);

    sda->stop();

    if (BTADDRESS[0] == 0) {
        QFAIL("Unable to find test service");
    }
    delete sda;
}
void tst_QBluetoothTransferManager::error(QBluetoothServiceDiscoveryAgent::Error error)
{
    qDebug() << "Received error" << error;
//    done_discovery = true;
}

void tst_QBluetoothTransferManager::finished()
{
    qDebug() << "Finished";
    done_discovery = true;
}

void tst_QBluetoothTransferManager::serviceDiscovered(const QBluetoothServiceInfo &info)
{
    qDebug() << "Found: " << info.device().name() << info.serviceUuid();
    strcpy(BTADDRESS, info.device().address().toString().toLatin1());
    done_discovery = true;
}

void tst_QBluetoothTransferManager::tst_construction()
{
    QBluetoothTransferManager *manager = new QBluetoothTransferManager();

    QVERIFY(manager);
    delete manager;
}

void tst_QBluetoothTransferManager::tst_put_data()
{
    QTest::addColumn<QBluetoothAddress>("address");
    QTest::addColumn<QMap<int, QVariant> >("parameters");

    QMap<int, QVariant> inparameters;
    inparameters.insert((int)QBluetoothTransferRequest::DescriptionAttribute, "Desciption");
    inparameters.insert((int)QBluetoothTransferRequest::LengthAttribute, QVariant(1024));
    inparameters.insert((int)QBluetoothTransferRequest::TypeAttribute, "OPP");

    QTest::newRow("0x000000 COD") << QBluetoothAddress(BTADDRESS) << inparameters;
}

void tst_QBluetoothTransferManager::tst_put()
{
    QFETCH(QBluetoothAddress, address);
    QFETCH(tst_QBluetoothTransferManager_QParameterMap, parameters);

    QBluetoothTransferRequest transferRequest(address);

    foreach (int key, parameters.keys()) {
        transferRequest.setAttribute((QBluetoothTransferRequest::Attribute)key, parameters[key]);
    }

    QBluetoothTransferManager manager;
}

void tst_QBluetoothTransferManager::tst_putAbort_data()
{
    QTest::addColumn<QBluetoothAddress>("address");
    QTest::addColumn<QMap<int, QVariant> >("parameters");

    QMap<int, QVariant> inparameters;
    inparameters.insert((int)QBluetoothTransferRequest::DescriptionAttribute, "Desciption");
    inparameters.insert((int)QBluetoothTransferRequest::LengthAttribute, QVariant(1024));
    inparameters.insert((int)QBluetoothTransferRequest::TypeAttribute, "OPP");

    QTest::newRow("0x000000 COD") << QBluetoothAddress(BTADDRESS) << inparameters;
}

void tst_QBluetoothTransferManager::tst_putAbort()
{
    QFETCH(QBluetoothAddress, address);
    QFETCH(tst_QBluetoothTransferManager_QParameterMap, parameters);

    QBluetoothTransferRequest transferRequest(address);

    foreach (int key, parameters.keys()) {
        transferRequest.setAttribute((QBluetoothTransferRequest::Attribute)key, parameters[key]);
    }

    QBluetoothTransferManager manager;
}

void tst_QBluetoothTransferManager::tst_attribute_data()
{
    QTest::addColumn<QBluetoothAddress>("address");
    QTest::addColumn<QMap<int, QVariant> >("parameters");

    QMap<int, QVariant> inparameters;
    inparameters.insert((int)QBluetoothTransferRequest::DescriptionAttribute, "Desciption");
    inparameters.insert((int)QBluetoothTransferRequest::LengthAttribute, QVariant(1024));
    inparameters.insert((int)QBluetoothTransferRequest::TypeAttribute, "OPP");

    QTest::newRow("0x000000 COD") << QBluetoothAddress(BTADDRESS) << inparameters;
}

void tst_QBluetoothTransferManager::tst_attribute()
{
    QFETCH(QBluetoothAddress, address);
    QFETCH(tst_QBluetoothTransferManager_QParameterMap, parameters);

    QBluetoothTransferRequest transferRequest(address);

    foreach (int key, parameters.keys()) {
        transferRequest.setAttribute((QBluetoothTransferRequest::Attribute)key, parameters[key]);
    }

    QBluetoothTransferManager manager;
}

void tst_QBluetoothTransferManager::tst_operation_data()
{
    QTest::addColumn<QBluetoothAddress>("address");

    QTest::newRow("0x000000 COD") << QBluetoothAddress(BTADDRESS);
}

void tst_QBluetoothTransferManager::tst_operation()
{
    QFETCH(QBluetoothAddress, address);

    QBluetoothTransferRequest transferRequest(address);
    QBluetoothTransferManager manager;
}

void tst_QBluetoothTransferManager::tst_manager_data()
{
    QTest::addColumn<QBluetoothAddress>("address");

    QTest::newRow("0x000000 COD") << QBluetoothAddress(BTADDRESS);
}

void tst_QBluetoothTransferManager::tst_manager()
{
    QFETCH(QBluetoothAddress, address);

    QBluetoothTransferRequest transferRequest(address);
    QBluetoothTransferManager manager;
}

QTEST_MAIN(tst_QBluetoothTransferManager)

#include "tst_qbluetoothtransfermanager.moc"
