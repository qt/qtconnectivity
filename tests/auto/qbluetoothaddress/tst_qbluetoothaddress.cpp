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

#include <qbluetoothaddress.h>

QTBLUETOOTH_USE_NAMESPACE

class tst_QBluetoothAddress : public QObject
{
    Q_OBJECT

public:
    tst_QBluetoothAddress();
    ~tst_QBluetoothAddress();

private slots:
    void tst_construction_data();
    void tst_construction();

    void tst_assignment();

    void tst_comparison_data();
    void tst_comparison();

    void tst_lessThan_data();
    void tst_lessThan();

    void tst_clear_data();
    void tst_clear();
};

tst_QBluetoothAddress::tst_QBluetoothAddress()
{
}

tst_QBluetoothAddress::~tst_QBluetoothAddress()
{
}

void tst_QBluetoothAddress::tst_construction_data()
{
    QTest::addColumn<quint64>("addressUInt");
    QTest::addColumn<QString>("addressS12");
    QTest::addColumn<QString>("addressS17");

    QTest::newRow("11:22:33:44:55:66") << Q_UINT64_C(0x112233445566) << QString("112233445566") << QString("11:22:33:44:55:66");
    QTest::newRow("AA:BB:CC:DD:EE:FF") << Q_UINT64_C(0xAABBCCDDEEFF) << QString("AABBCCDDEEFF") << QString("AA:BB:CC:DD:EE:FF");
    QTest::newRow("aa:bb:cc:dd:ee:ff") << Q_UINT64_C(0xAABBCCDDEEFF) << QString("aabbccddeeff") << QString("AA:BB:CC:DD:EE:FF");
    QTest::newRow("FF:FF:FF:FF:FF:FF") << Q_UINT64_C(0xFFFFFFFFFFFF) << QString("FFFFFFFFFFFF") << QString("FF:FF:FF:FF:FF:FF");
}

void tst_QBluetoothAddress::tst_construction()
{
    QFETCH(quint64, addressUInt);
    QFETCH(QString, addressS12);
    QFETCH(QString, addressS17);

    {
        QBluetoothAddress address;

        QVERIFY(address.isNull());
    }

    {
        /* construct from quint64 */
        QBluetoothAddress address(addressUInt);

        QVERIFY(!address.isNull());

        QVERIFY(address.toUInt64() == addressUInt);

        QCOMPARE(address.toString(), addressS17);
    }

    {
        /* construct from string without colons */
        QBluetoothAddress address(addressS12);

        QVERIFY(!address.isNull());

        QVERIFY(address.toUInt64() == addressUInt);

        QCOMPARE(address.toString(), addressS17);
    }

    {
        /* construct from string with colons */
        QBluetoothAddress address(addressS17);

        QVERIFY(!address.isNull());

        QVERIFY(address.toUInt64() == addressUInt);

        QCOMPARE(address.toString(), addressS17);
    }

    {
        QString empty;
        QBluetoothAddress address(empty);

        QVERIFY(address.isNull());
    }

    {
        QBluetoothAddress address(addressUInt);

        QBluetoothAddress copy(address);

        QVERIFY(address.toUInt64() == copy.toUInt64());
    }
}

void tst_QBluetoothAddress::tst_assignment()
{
    QBluetoothAddress address(Q_UINT64_C(0x112233445566));

    {
        QBluetoothAddress copy = address;

        QCOMPARE(address.toUInt64(), copy.toUInt64());
    }

    {
        QBluetoothAddress copy1;
        QBluetoothAddress copy2;

        QVERIFY(copy1.isNull());
        QVERIFY(copy2.isNull());

        copy1 = copy2 = address;

        QVERIFY(!copy1.isNull());
        QVERIFY(!copy2.isNull());

        QVERIFY(address.toUInt64() == copy1.toUInt64());
        QVERIFY(address.toUInt64() == copy2.toUInt64());

        copy1.clear();
        QVERIFY(copy1.isNull());
        QVERIFY2(copy1 != address, "Verify that copy1 is a copy of address, the d_ptr are being copied");
    }
}

void tst_QBluetoothAddress::tst_comparison_data()
{
    QTest::addColumn<QBluetoothAddress>("address1");
    QTest::addColumn<QBluetoothAddress>("address2");
    QTest::addColumn<bool>("result");

    QTest::newRow("invalid == invalid") << QBluetoothAddress() << QBluetoothAddress() << true;
    QTest::newRow("valid != invalid") << QBluetoothAddress(Q_UINT64_C(0x112233445566)) << QBluetoothAddress() << false;
    QTest::newRow("valid == valid") << QBluetoothAddress(Q_UINT64_C(0x112233445566)) << QBluetoothAddress(Q_UINT64_C(0x112233445566)) << true;
}

void tst_QBluetoothAddress::tst_comparison()
{
    QFETCH(QBluetoothAddress, address1);
    QFETCH(QBluetoothAddress, address2);
    QFETCH(bool, result);

    QCOMPARE(address1 == address2, result);
    QCOMPARE(address2 == address1, result);
    QCOMPARE(address1 != address2, !result);
    QCOMPARE(address2 != address1, !result);
}

void tst_QBluetoothAddress::tst_lessThan_data()
{
    QTest::addColumn<QBluetoothAddress>("address1");
    QTest::addColumn<QBluetoothAddress>("address2");
    QTest::addColumn<bool>("result");

    QTest::newRow("invalid < invalid") << QBluetoothAddress() << QBluetoothAddress() << false;
    QTest::newRow("invalid < valid") << QBluetoothAddress() << QBluetoothAddress(Q_UINT64_C(0x112233445566)) << true;
    QTest::newRow("valid < invalid") << QBluetoothAddress(Q_UINT64_C(0x112233445566)) << QBluetoothAddress() << false;
    QTest::newRow("valid < valid") << QBluetoothAddress(Q_UINT64_C(0x112233445566)) << QBluetoothAddress(Q_UINT64_C(0xAABBCCDDEEFF)) << true;
    QTest::newRow("valid < valid") << QBluetoothAddress(Q_UINT64_C(0xAABBCCDDEEFF)) << QBluetoothAddress(Q_UINT64_C(0x112233445566)) << false;
}

void tst_QBluetoothAddress::tst_lessThan()
{
    QFETCH(QBluetoothAddress, address1);
    QFETCH(QBluetoothAddress, address2);
    QFETCH(bool, result);

    QCOMPARE(address1 < address2, result);
}

void tst_QBluetoothAddress::tst_clear_data()
{
    QTest::addColumn<QString>("addressS17");

    QTest::newRow("FF:00:F3:25:00:00") << QString("FF:00:F3:25:00:00");
}

void tst_QBluetoothAddress::tst_clear()
{
    QFETCH(QString, addressS17);

    QBluetoothAddress address(addressS17);
    QVERIFY(!address.isNull());
    address.clear();
    QVERIFY(address.toString() == QString("00:00:00:00:00:00"));
}

QTEST_MAIN(tst_QBluetoothAddress)

#include "tst_qbluetoothaddress.moc"

