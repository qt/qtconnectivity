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
#include <QString>

#include <qbluetoothhostinfo.h>

QTBLUETOOTH_USE_NAMESPACE

class tst_QBluetoothHostInfo : public QObject
{
    Q_OBJECT

public:
    tst_QBluetoothHostInfo();
    ~tst_QBluetoothHostInfo();

private slots:
    void tst_address_data();
    void tst_address();

    void tst_name_data();
    void tst_name();

    void tst_construction_data();
    void tst_construction();
};

tst_QBluetoothHostInfo::tst_QBluetoothHostInfo()
{
}

tst_QBluetoothHostInfo::~tst_QBluetoothHostInfo()
{
}

void tst_QBluetoothHostInfo::tst_address()
{
    QFETCH(QString, addressString);

    QBluetoothAddress address(addressString);
    QVERIFY(!address.isNull());
    QCOMPARE(address.toString(), addressString);

    QBluetoothHostInfo info;
    QBluetoothAddress result = info.getAddress();
    QVERIFY(result.isNull());
    info.setAddress(address);
    QCOMPARE(info.getAddress().toString(), addressString);

}

void tst_QBluetoothHostInfo::tst_address_data()
{
    QTest::addColumn<QString>("addressString");

    QTest::newRow("11:22:33:44:55:66") << QString("11:22:33:44:55:66");
    QTest::newRow("AA:BB:CC:DD:EE:FF") << QString("AA:BB:CC:DD:EE:FF");
    QTest::newRow("aa:bb:cc:dd:ee:ff") << QString("AA:BB:CC:DD:EE:FF");
    QTest::newRow("FF:FF:FF:FF:FF:FF") << QString("FF:FF:FF:FF:FF:FF");
}

void tst_QBluetoothHostInfo::tst_name()
{
    QFETCH(QString, name);

    QBluetoothHostInfo info;
    QString result = info.getName();
    QVERIFY(result.isNull());
    QVERIFY(result.isEmpty());

    info.setName(name);
    QCOMPARE(info.getName(), name);
}

void tst_QBluetoothHostInfo::tst_name_data()
{
    QTest::addColumn<QString>("name");

    QTest::newRow("empty/default name") << QString();
    QTest::newRow("empty name") << QString("");
    QTest::newRow("ABCD") << QString("ABCD");
    QTest::newRow("Very long name") << QString("ThisIsAVeryLongNameString-abcdefghijklmnopqrstuvwxyz");
    QTest::newRow("special chars") << QString("gh\nfg i-+.,/;");
}

void tst_QBluetoothHostInfo::tst_construction_data()
{
    QTest::addColumn<QString>("btAddress");
    QTest::addColumn<QString>("name");
    QTest::addColumn<bool>("validBtAddress");

    QTest::newRow("11:22:33:44:55:66") << QString("11:22:33:44:55:66") << QString() << true;
    QTest::newRow("AA:BB:CC:DD:EE:FF") << QString("AA:BB:CC:DD:EE:FF") << QString("") << true;
    QTest::newRow("aa:bb:cc:dd:ee:ff") << QString("AA:BB:CC:DD:EE:FF") << QString("foobar") << true;
    QTest::newRow("FF:FF:FF:FF:FF:FF") << QString("FF:FF:FF:FF:FF:FF") << QString("WeUseAlongStringAsName_FFFFFFFFFFFFFFFFFFFF") << true;
    QTest::newRow("00:00:00:00:00:00") << QString("00:00:00:00:00:00") << QString("foobar2") << false;
}

void tst_QBluetoothHostInfo::tst_construction()
{
    QFETCH(QString, btAddress);
    QFETCH(QString, name);
    QFETCH(bool, validBtAddress);

    QBluetoothAddress empty;
    QVERIFY(empty.isNull());

    QBluetoothHostInfo setter;
    QBluetoothAddress addr(btAddress);
    setter.setName(name);
    setter.setAddress(addr);
    QCOMPARE(setter.getName(), name);
    QCOMPARE(setter.getAddress().toString(), btAddress);
    QCOMPARE(setter.getAddress().isNull(), !validBtAddress);

    setter.setAddress(empty);
    QCOMPARE(setter.getName(), name);
    QCOMPARE(setter.getAddress().toString(), QString("00:00:00:00:00:00"));
    QCOMPARE(setter.getAddress().isNull(), true);

    setter.setName(QString());
    QCOMPARE(setter.getName(), QString());
    QCOMPARE(setter.getAddress().toString(), QString("00:00:00:00:00:00"));
    QCOMPARE(setter.getAddress().isNull(), true);

    setter.setAddress(addr);
    QCOMPARE(setter.getName(), QString());
    QCOMPARE(setter.getAddress().toString(), btAddress);
    QCOMPARE(setter.getAddress().isNull(), !validBtAddress);
}

QTEST_MAIN(tst_QBluetoothHostInfo)

#include "tst_qbluetoothhostinfo.moc"
