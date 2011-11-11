/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
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
#include <QUuid>

#include <QDebug>

#include <qbluetoothuuid.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#ifdef Q_OS_SYMBIAN
//needed for symbian uuid conversion test
#include <bttypes.h>
#endif

QTBLUETOOTH_USE_NAMESPACE

Q_DECLARE_METATYPE(quint128)

class tst_QBluetoothUuid : public QObject
{
    Q_OBJECT

public:
    tst_QBluetoothUuid();
    ~tst_QBluetoothUuid();

private slots:
    void initTestCase();

    void tst_construction();
    void tst_assignment();
    void tst_conversion_data();
    void tst_conversion();
    void tst_comparison_data();
    void tst_comparison();
#ifdef Q_OS_SYMBIAN
    void tst_symbian_uuid_tranformation_data();
    void tst_symbian_uuid_tranformation();
#endif
};

tst_QBluetoothUuid::tst_QBluetoothUuid()
{
    qRegisterMetaType<quint128>("quint128");
}

tst_QBluetoothUuid::~tst_QBluetoothUuid()
{
}

void tst_QBluetoothUuid::initTestCase()
{
}

void tst_QBluetoothUuid::tst_construction()
{
    {
        QBluetoothUuid nullUuid;

        QVERIFY(nullUuid.isNull());
    }

    {
        QBluetoothUuid uuid(QBluetoothUuid::PublicBrowseGroup);

        QVERIFY(!uuid.isNull());

        bool ok;
        quint16 uuid16;

        uuid16 = uuid.toUInt16(&ok);

        QVERIFY(ok);
        QCOMPARE(uuid16, static_cast<quint16>(QBluetoothUuid::PublicBrowseGroup));
    }

    {
        QBluetoothUuid uuid(QBluetoothUuid::PublicBrowseGroup);

        QBluetoothUuid copy(uuid);

        QCOMPARE(uuid.toUInt16(), copy.toUInt16());
    }

    {
        QBluetoothUuid uuid(QBluetoothUuid::L2cap);

        QVERIFY(!uuid.isNull());

        bool ok;
        quint16 uuid16;

        uuid16 = uuid.toUInt16(&ok);

        QVERIFY(ok);
        QCOMPARE(uuid16, static_cast<quint16>(QBluetoothUuid::L2cap));
    }

    {
        QUuid uuid(0x67c8770b, 0x44f1, 0x410a, 0xab, 0x9a, 0xf9, 0xb5, 0x44, 0x6f, 0x13, 0xee);
        QBluetoothUuid btUuid(uuid);
        QVERIFY(!btUuid.isNull());

        QString uuidString(btUuid.toString());
        QVERIFY(!uuidString.isEmpty());
        QCOMPARE(uuidString, QString("{67c8770b-44f1-410a-ab9a-f9b5446f13ee}"));
    }

    {
        QBluetoothUuid btUuid(QString("67c8770b-44f1-410a-ab9a-f9b5446f13ee"));
        QVERIFY(!btUuid.isNull());

        QString uuidString(btUuid.toString());
        QVERIFY(!uuidString.isEmpty());
        QCOMPARE(uuidString, QString("{67c8770b-44f1-410a-ab9a-f9b5446f13ee}"));
    }

    {
        QBluetoothUuid btUuid(QString("XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX"));
        QVERIFY(btUuid.isNull());
    }
}

void tst_QBluetoothUuid::tst_assignment()
{
    QBluetoothUuid uuid(QBluetoothUuid::PublicBrowseGroup);

    {
        QBluetoothUuid copy = uuid;

        QCOMPARE(uuid.toUInt16(), copy.toUInt16());
    }

    {
        QBluetoothUuid copy1;
        QBluetoothUuid copy2;

        QVERIFY(copy1.isNull());
        QVERIFY(copy2.isNull());

        copy1 = copy2 = uuid;

        QVERIFY(!copy1.isNull());
        QVERIFY(!copy2.isNull());

        QCOMPARE(uuid.toUInt16(), copy1.toUInt16());
        QCOMPARE(uuid.toUInt16(), copy2.toUInt16());
    }
}

#define BASEUUID "-0000-1000-8000-00805F9B34FB"

#define UUID128_32(x, a, b, c, d) \
    quint128 x = { \
        { \
            a, b, c, d, \
            0x00, 0x00, \
            0x10, 0x00, \
            0x80, 0x00, \
            0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB \
        } \
    }

#define UUID128_16(x, a, b) UUID128_32(x, 0, 0, a, b)

#define NEWROW16(text, a, b, s) \
do { \
    UUID128_16(uuid128, a, b); \
    quint32 uuid32 = a << 8 | b; \
    quint16 uuid16 = a << 8 | b; \
    QTest::newRow(text) << true << uuid16 << true << uuid32 << true << uuid128 \
                        << (QLatin1Char('{') + QLatin1String(s) + QLatin1Char('}')); \
} while (0)

#define NEWROW32(text, a, b, c, d, s) \
do { \
    UUID128_32(uuid128, a, b, c, d); \
    quint32 uuid32 = a << 24 | b << 16 | c << 8 | d; \
    quint16 uuid16; \
    bool constructUuid16 = (a == 0) && (b == 0); \
    if (constructUuid16) \
        uuid16 = c << 8 | d; \
    else \
        uuid16 = 0; \
    QTest::newRow(text) << constructUuid16 << uuid16 << true << uuid32 << true << uuid128 \
                        << (QLatin1Char('{') + QLatin1String(s) + QLatin1Char('}')); \
} while (0)

void tst_QBluetoothUuid::tst_conversion_data()
{
    QTest::addColumn<bool>("constructUuid16");
    QTest::addColumn<quint16>("uuid16");
    QTest::addColumn<bool>("constructUuid32");
    QTest::addColumn<quint32>("uuid32");
    QTest::addColumn<bool>("constructUuid128");
    QTest::addColumn<quint128>("uuid128");
    QTest::addColumn<QString>("uuidS");

    NEWROW32("base uuid", 0x00, 0x00, 0x00, 0x00, "00000000" BASEUUID);
    NEWROW16("0x0001", 0x00, 0x01, "00000001" BASEUUID);
    NEWROW16("0xffff", 0xff, 0xff, "0000FFFF" BASEUUID);
    NEWROW32("0x00010000", 0x00, 0x01, 0x00, 0x00, "00010000" BASEUUID);
    NEWROW32("0x0001ffff", 0x00, 0x01, 0xff, 0xff, "0001FFFF" BASEUUID);
    NEWROW32("0xffff0000", 0xff, 0xff, 0x00, 0x00, "FFFF0000" BASEUUID);
    NEWROW32("0xffffffff", 0xff, 0xff, 0xff, 0xff, "FFFFFFFF" BASEUUID);

    {
        quint128 uuid128 = {
            {
                0x00, 0x11, 0x22, 0x33,
                0x44, 0x55,
                0x66, 0x77,
                0x88, 0x99,
                0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
            }
        };

        QTest::newRow("00112233-4455-6677-8899-AABBCCDDEEFF")
            << false << quint16(0) << false << quint32(0) << true << uuid128
            << QString(QLatin1String("{00112233-4455-6677-8899-AABBCCDDEEFF}"));
    }
}

void tst_QBluetoothUuid::tst_conversion()
{
    QFETCH(bool, constructUuid16);
    QFETCH(quint16, uuid16);
    QFETCH(bool, constructUuid32);
    QFETCH(quint32, uuid32);
    QFETCH(bool, constructUuid128);
    QFETCH(quint128, uuid128);
    QFETCH(QString, uuidS);

    int minimumSize = 16;
    if (constructUuid16)
        minimumSize = 2;
    else if (constructUuid32)
        minimumSize = 4;

    if (constructUuid16) {
        QBluetoothUuid uuid(uuid16);

        bool ok;

        QCOMPARE(uuid.toUInt16(&ok), uuid16);
        QVERIFY(ok);

        QCOMPARE(uuid.toUInt32(&ok), uuid32);
        QVERIFY(ok);

        QVERIFY(memcmp(uuid.toUInt128().data, uuid128.data, 16) == 0);

        QCOMPARE(uuid.toString().toUpper(), uuidS.toUpper());

        QCOMPARE(uuid.minimumSize(), minimumSize);
    }

    if (constructUuid32) {
        QBluetoothUuid uuid(uuid32);

        bool ok;

        quint16 tmp = uuid.toUInt16(&ok);
        QCOMPARE(ok, constructUuid16);
        if (ok) {
            QCOMPARE(tmp, uuid16);
        }

        QCOMPARE(uuid.toUInt32(&ok), uuid32);
        QVERIFY(ok);

        QVERIFY(memcmp(uuid.toUInt128().data, uuid128.data, 16) == 0);

        QCOMPARE(uuid.toString().toUpper(), uuidS.toUpper());

        QCOMPARE(uuid.minimumSize(), minimumSize);
    }

    if (constructUuid128) {
        QBluetoothUuid uuid(uuid128);

        bool ok;

        quint16 tmpUuid16 = uuid.toUInt16(&ok);
        QCOMPARE(ok, constructUuid16);
        if (ok)
            QCOMPARE(tmpUuid16, uuid16);

        quint32 tmpUuid32 = uuid.toUInt32(&ok);
        QCOMPARE(ok, constructUuid32);
        if (ok)
            QCOMPARE(tmpUuid32, uuid32);

        QVERIFY(memcmp(uuid.toUInt128().data, uuid128.data, 16) == 0);

        QCOMPARE(uuid.toString().toUpper(), uuidS.toUpper());

        QCOMPARE(uuid.minimumSize(), minimumSize);
    }
}

void tst_QBluetoothUuid::tst_comparison_data()
{
    tst_conversion_data();
}

void tst_QBluetoothUuid::tst_comparison()
{
    QFETCH(bool, constructUuid16);
    QFETCH(quint16, uuid16);
    QFETCH(bool, constructUuid32);
    QFETCH(quint32, uuid32);
    QFETCH(bool, constructUuid128);
    QFETCH(quint128, uuid128);

    QVERIFY(QBluetoothUuid() == QBluetoothUuid());

    if (constructUuid16) {
        QBluetoothUuid quuid16(uuid16);
        QBluetoothUuid quuid32(uuid32);
        QBluetoothUuid quuid128(uuid128);

        QVERIFY(quuid16.toUInt16() == uuid16);
        QVERIFY(quuid16.toUInt32() == uuid32);

        QVERIFY(quuid16 == quuid32);
        QVERIFY(quuid16 == quuid128);

        QVERIFY(quuid32 == quuid16);
        QVERIFY(quuid128 == quuid16);
    }

    if (constructUuid32) {
        QBluetoothUuid quuid32(uuid32);
        QBluetoothUuid quuid128(uuid128);

        QVERIFY(quuid32.toUInt32() == uuid32);
        QVERIFY(quuid32 == quuid128);

        QVERIFY(quuid128 == quuid32);
    }

    if (constructUuid128) {
        QBluetoothUuid quuid128(uuid128);

        for (int var = 0; var < 16; ++var) {
            QVERIFY(quuid128.toUInt128().data[var] == uuid128.data[var]);
        }
    }
}
#ifdef Q_OS_SYMBIAN
void tst_QBluetoothUuid::tst_symbian_uuid_tranformation_data()
{
    tst_conversion_data();
}

void tst_QBluetoothUuid::tst_symbian_uuid_tranformation()
{
    QFETCH(quint128, uuid128);

    QBluetoothUuid quuid128(uuid128);

    for (int var = 0; var < 16; ++var) {
        QVERIFY(quuid128.toUInt128().data[var] == uuid128.data[var]);
    }

    if (quuid128.minimumSize() == 2) {
        TUUID sUuid(quuid128.toUInt16());
        QBluetoothUuid fromSymbianUuid(ntohs(*reinterpret_cast<const quint16 *>(sUuid.Des().Ptr())));
        QCOMPARE(quuid128, fromSymbianUuid);
        //qDebug()<<"sUuid size  ="<<sUuid.Des().Length() ;
    } else if (quuid128.minimumSize() == 4) {
        TUUID sUuid(quuid128.toUInt32());
        QBluetoothUuid fromSymbianUuid(ntohl(*reinterpret_cast<const quint32 *>(sUuid.Des().Ptr())));
        QCOMPARE(quuid128, fromSymbianUuid);
        //qDebug()<<"sUuid size  ="<<sUuid.Des().Length() ;
    } else if (quuid128.minimumSize()==16) {

        TUint32 *dataPointer = (TUint32*)quuid128.toUInt128().data;
        TUint32 hH = qToBigEndian<quint32>(*(dataPointer++));
        TUint32 hL = qToBigEndian<quint32>(*(dataPointer++));
        TUint32 lH = qToBigEndian<quint32>(*(dataPointer++));
        TUint32 lL = qToBigEndian<quint32>(*(dataPointer));
        TUUID sUuid(hH, hL, lH, lL);
        //qDebug()<<"sUuid size      ="<<sUuid.Des().Length() ;
        //qDebug()<<"sUuid longsize  ="<<sUuid.LongForm().Length() ;

        for (int var = 0; var < sUuid.Des().Length(); ++var) {
            //qDebug()<<"sUuid at    ="<<var<<"="<<sUuid.Des()[var] ;
            //qDebug()<<"quuid128 at ="<<var<<"="<<quuid128.toUInt128().data[var] ;
            QVERIFY(quuid128.toUInt128().data[var] == sUuid.Des()[var]);
        }
    }
}
#endif //Q_OS_SYMBIAN
QTEST_MAIN(tst_QBluetoothUuid)

#include "tst_qbluetoothuuid.moc"
