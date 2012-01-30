/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the QtNfc module of the Qt Toolkit.
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

#include <qndefrecord.h>
#include <qndefnfctextrecord.h>
#include <qndefnfcurirecord.h>

QTNFC_USE_NAMESPACE

class tst_QNdefRecord : public QObject
{
    Q_OBJECT

public:
    tst_QNdefRecord();
    ~tst_QNdefRecord();

private slots:
    void tst_record();

    void tst_textRecord_data();
    void tst_textRecord();

    void tst_uriRecord_data();
    void tst_uriRecord();
};

tst_QNdefRecord::tst_QNdefRecord()
{
}

tst_QNdefRecord::~tst_QNdefRecord()
{
}

void tst_QNdefRecord::tst_record()
{
    // test empty record
    {
        QNdefRecord record;

        QVERIFY(record.isEmpty());
        QCOMPARE(record.typeNameFormat(), QNdefRecord::Empty);
        QVERIFY(record.type().isEmpty());
        QVERIFY(record.id().isEmpty());
        QVERIFY(record.payload().isEmpty());

        QVERIFY(!record.isRecordType<QNdefNfcTextRecord>());
        QVERIFY(!record.isRecordType<QNdefNfcUriRecord>());

        QCOMPARE(record, QNdefRecord());
        QVERIFY(!(record != QNdefRecord()));
    }

    // test type name format
    {
        QNdefRecord record;

        record.setTypeNameFormat(QNdefRecord::Empty);
        QCOMPARE(record.typeNameFormat(), QNdefRecord::Empty);

        record.setTypeNameFormat(QNdefRecord::NfcRtd);
        QCOMPARE(record.typeNameFormat(), QNdefRecord::NfcRtd);

        record.setTypeNameFormat(QNdefRecord::Mime);
        QCOMPARE(record.typeNameFormat(), QNdefRecord::Mime);

        record.setTypeNameFormat(QNdefRecord::Uri);
        QCOMPARE(record.typeNameFormat(), QNdefRecord::Uri);

        record.setTypeNameFormat(QNdefRecord::ExternalRtd);
        QCOMPARE(record.typeNameFormat(), QNdefRecord::ExternalRtd);

        record.setTypeNameFormat(QNdefRecord::Unknown);
        QCOMPARE(record.typeNameFormat(), QNdefRecord::Unknown);

        record.setTypeNameFormat(QNdefRecord::TypeNameFormat(6));
        QCOMPARE(record.typeNameFormat(), QNdefRecord::Unknown);

        record.setTypeNameFormat(QNdefRecord::TypeNameFormat(7));
        QCOMPARE(record.typeNameFormat(), QNdefRecord::Unknown);
    }

    // test type
    {
        QNdefRecord record;

        record.setType("test type");
        QCOMPARE(record.type(), QByteArray("test type"));
    }

    // test id
    {
        QNdefRecord record;

        record.setId("test id");
        QCOMPARE(record.id(), QByteArray("test id"));
    }

    // test payload
    {
        QNdefRecord record;

        record.setPayload("test payload");
        QVERIFY(!record.isEmpty());
        QVERIFY(!record.payload().isEmpty());
        QCOMPARE(record.payload(), QByteArray("test payload"));
    }

    // test copy
    {
        QNdefRecord record;
        record.setTypeNameFormat(QNdefRecord::ExternalRtd);
        record.setType("qt.nokia.com:test-rtd");
        record.setId("test id");
        record.setPayload("test payload");

        QNdefRecord ccopy(record);

        QCOMPARE(record.typeNameFormat(), ccopy.typeNameFormat());
        QCOMPARE(record.type(), ccopy.type());
        QCOMPARE(record.id(), ccopy.id());
        QCOMPARE(record.payload(), ccopy.payload());

        QVERIFY(record == ccopy);
        QVERIFY(!(record != ccopy));

        QNdefRecord acopy;
        acopy = record;

        QCOMPARE(record.typeNameFormat(), acopy.typeNameFormat());
        QCOMPARE(record.type(), acopy.type());
        QCOMPARE(record.id(), acopy.id());
        QCOMPARE(record.payload(), acopy.payload());

        QVERIFY(record == acopy);
        QVERIFY(!(record != acopy));

        QVERIFY(record != QNdefRecord());
    }

    // test comparison
    {
        QNdefRecord record;
        record.setTypeNameFormat(QNdefRecord::ExternalRtd);
        record.setType("qt.nokia.com:test-rtd");
        record.setId("test id");
        record.setPayload("test payload");

        QNdefRecord other;
        other.setTypeNameFormat(QNdefRecord::ExternalRtd);
        other.setType("qt.nokia.com:test-other-rtd");
        other.setId("test other id");
        other.setPayload("test other payload");

        QVERIFY(record != other);
    }
}

void tst_QNdefRecord::tst_textRecord_data()
{
    QTest::addColumn<QString>("locale");
    QTest::addColumn<QString>("text");
    QTest::addColumn<bool>("utf8");
    QTest::addColumn<QByteArray>("payload");


    QTest::newRow("en_US utf8") << QString::fromLatin1("en_US")
                                << QString::fromLatin1("Test String")
                                << true
                                << QByteArray::fromHex("05656E5F55535465737420537472696E67");
    QTest::newRow("en_US utf16") << QString::fromLatin1("en_US")
                                 << QString::fromLatin1("Test String")
                                 << false
                                 << QByteArray::fromHex("85656E5F5553FEFF005400650073007400200053007400720069006E0067");
    QTest::newRow("ja_JP utf8") << QString::fromLatin1("ja_JP")
                                << QString::fromUtf8("\343\203\206\343\202\271\343\203\210\346"
                                                     "\226\207\345\255\227\345\210\227")
                                << true
                                << QByteArray::fromHex("056A615F4A50E38386E382B9E38388E69687E5AD97E58897");
    QTest::newRow("ja_JP utf16") << QString::fromLatin1("ja_JP")
                                 << QString::fromUtf8("\343\203\206\343\202\271\343\203\210\346"
                                                      "\226\207\345\255\227\345\210\227")
                                 << false
                                 << QByteArray::fromHex("856A615F4A50FEFF30C630B930C865875B575217");
}

void tst_QNdefRecord::tst_textRecord()
{
    QFETCH(QString, locale);
    QFETCH(QString, text);
    QFETCH(bool, utf8);
    QFETCH(QByteArray, payload);

    // test setters
    {
        QNdefNfcTextRecord record;
        record.setLocale(locale);
        record.setText(text);
        record.setEncoding(utf8 ? QNdefNfcTextRecord::Utf8 : QNdefNfcTextRecord::Utf16);

        QCOMPARE(record.payload(), payload);

        QVERIFY(record != QNdefRecord());
    }

    // test getters
    {
        QNdefNfcTextRecord record;
        record.setPayload(payload);

        QCOMPARE(record.locale(), locale);
        QCOMPARE(record.text(), text);
        QCOMPARE(record.encoding() == QNdefNfcTextRecord::Utf8, utf8);
    }

    // test copy
    {
        QNdefRecord record;
        record.setTypeNameFormat(QNdefRecord::NfcRtd);
        record.setType("T");
        record.setPayload(payload);

        QVERIFY(!record.isRecordType<QNdefRecord>());
        QVERIFY(record.isRecordType<QNdefNfcTextRecord>());
        QVERIFY(!record.isRecordType<QNdefNfcUriRecord>());

        QNdefNfcTextRecord textRecord(record);

        QVERIFY(!textRecord.isEmpty());

        QVERIFY(record == textRecord);

        QCOMPARE(textRecord.locale(), locale);
        QCOMPARE(textRecord.text(), text);
        QCOMPARE(textRecord.encoding() == QNdefNfcTextRecord::Utf8, utf8);
    }
}

void tst_QNdefRecord::tst_uriRecord_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("payload");


    QTest::newRow("http") << QString::fromLatin1("http://qt.nokia.com/")
                                << QByteArray::fromHex("0371742E6E6F6B69612E636F6D2F");
    QTest::newRow("tel") << QString::fromLatin1("tel:+1234567890")
                         << QByteArray::fromHex("052B31323334353637383930");
    QTest::newRow("mailto") << QString::fromLatin1("mailto:test@example.com")
                            << QByteArray::fromHex("0674657374406578616D706C652E636F6D");
    QTest::newRow("urn") << QString::fromLatin1("urn:nfc:ext:qt.nokia.com:test")
                         << QByteArray::fromHex("136E66633A6578743A71742E6E6F6B69612E636F6D3A74657374");
}

void tst_QNdefRecord::tst_uriRecord()
{
    QFETCH(QString, url);
    QFETCH(QByteArray, payload);

    // test setters
    {
        QNdefNfcUriRecord record;
        record.setUri(QUrl(url));

        QCOMPARE(record.payload(), payload);

        QVERIFY(record != QNdefRecord());
    }

    // test getters
    {
        QNdefNfcUriRecord record;
        record.setPayload(payload);

        QCOMPARE(record.uri(), QUrl(url));
    }

    // test copy
    {
        QNdefRecord record;
        record.setTypeNameFormat(QNdefRecord::NfcRtd);
        record.setType("U");
        record.setPayload(payload);

        QVERIFY(!record.isRecordType<QNdefRecord>());
        QVERIFY(!record.isRecordType<QNdefNfcTextRecord>());
        QVERIFY(record.isRecordType<QNdefNfcUriRecord>());

        QNdefNfcUriRecord uriRecord(record);

        QVERIFY(!uriRecord.isEmpty());

        QVERIFY(record == uriRecord);

        QCOMPARE(uriRecord.uri(), QUrl(url));
    }
}

QTEST_MAIN(tst_QNdefRecord)

#include "tst_qndefrecord.moc"

