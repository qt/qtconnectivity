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
#include <QMap>

#include <qbluetoothtransferrequest.h>
#include <qbluetoothaddress.h>
#include <qbluetoothlocaldevice.h>

QTCONNECTIVITY_USE_NAMESPACE

typedef QMap<int,QVariant> tst_QBluetoothTransferRequest_QParameterMap;
Q_DECLARE_METATYPE(tst_QBluetoothTransferRequest_QParameterMap)

class tst_QBluetoothTransferRequest : public QObject
{
    Q_OBJECT

public:
    tst_QBluetoothTransferRequest();
    ~tst_QBluetoothTransferRequest();

private slots:
    void initTestCase();

    void tst_construction_data();
    void tst_construction();

    void tst_assignment_data();
    void tst_assignment();
};

tst_QBluetoothTransferRequest::tst_QBluetoothTransferRequest()
{
}

tst_QBluetoothTransferRequest::~tst_QBluetoothTransferRequest()
{
}

void tst_QBluetoothTransferRequest::initTestCase()
{
    // start Bluetooth if not started
    QBluetoothLocalDevice *device = new QBluetoothLocalDevice();
    device->powerOn();
    delete device;
}

void tst_QBluetoothTransferRequest::tst_construction_data()
{
    QTest::addColumn<QBluetoothAddress>("address");
    QTest::addColumn<QMap<int, QVariant> >("parameters");

    QMap<int, QVariant> inparameters;
    inparameters.insert((int)QBluetoothTransferRequest::DescriptionAttribute, "Desciption");
    inparameters.insert((int)QBluetoothTransferRequest::LengthAttribute, QVariant(1024));
    inparameters.insert((int)QBluetoothTransferRequest::TypeAttribute, "OPP");

    QTest::newRow("0x000000 COD") << QBluetoothAddress("000000000000") << inparameters;
    QTest::newRow("0x000100 COD") << QBluetoothAddress("000000000000") << inparameters;
    QTest::newRow("0x000104 COD") << QBluetoothAddress("000000000000") << inparameters;
    QTest::newRow("0x000118 COD") << QBluetoothAddress("000000000000") << inparameters;
    QTest::newRow("0x000200 COD") << QBluetoothAddress("000000000000") << inparameters;
}

void tst_QBluetoothTransferRequest::tst_construction()
{
    QFETCH(QBluetoothAddress, address);
    QFETCH(tst_QBluetoothTransferRequest_QParameterMap, parameters);

    QBluetoothTransferRequest transferRequest(address);

    foreach (int key, parameters.keys()) {
        transferRequest.setAttribute((QBluetoothTransferRequest::Attribute)key, parameters[key]);
        QCOMPARE(parameters[key], transferRequest.attribute((QBluetoothTransferRequest::Attribute)key));
    }

    QCOMPARE(transferRequest.address(), address);

    QBluetoothTransferRequest copyRequest(transferRequest);

    QCOMPARE(copyRequest.address(), address);
    QCOMPARE(transferRequest, copyRequest);
}

void tst_QBluetoothTransferRequest::tst_assignment_data()
{
    tst_construction_data();
}

void tst_QBluetoothTransferRequest::tst_assignment()
{
    QFETCH(QBluetoothAddress, address);
    QFETCH(tst_QBluetoothTransferRequest_QParameterMap, parameters);

    QBluetoothTransferRequest transferRequest(address);

    foreach (int key, parameters.keys()) {
        transferRequest.setAttribute((QBluetoothTransferRequest::Attribute)key, parameters[key]);
    }

    {
        QBluetoothTransferRequest copyRequest = transferRequest;

        QCOMPARE(copyRequest.address(), address);
        QCOMPARE(transferRequest, copyRequest);
    }

    {
        QBluetoothTransferRequest copyRequest(QBluetoothAddress("000000000001"));

        copyRequest = transferRequest;

        QCOMPARE(copyRequest.address(), address);
        QCOMPARE(transferRequest, copyRequest);
    }

    {
        QBluetoothTransferRequest copyRequest1(QBluetoothAddress("000000000001"));
        QBluetoothTransferRequest copyRequest2(QBluetoothAddress("000000000001"));

        copyRequest1 = copyRequest2 = transferRequest;

        QCOMPARE(copyRequest1.address(), address);
        QCOMPARE(copyRequest2.address(), address);
        QCOMPARE(transferRequest, copyRequest1);
        QCOMPARE(transferRequest, copyRequest2);

    }
}

QTEST_MAIN(tst_QBluetoothTransferRequest)

#include "tst_qbluetoothtransferrequest.moc"
