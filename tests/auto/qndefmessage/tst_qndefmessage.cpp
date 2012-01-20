/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
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
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtTest/QtTest>

#include <qndefrecord.h>
#include <qndefmessage.h>
#include <qndefnfctextrecord.h>
#include <qndefnfcurirecord.h>

QTNFC_USE_NAMESPACE

Q_DECLARE_METATYPE(QNdefRecord)

class tst_QNdefMessage : public QObject
{
    Q_OBJECT

public:
    tst_QNdefMessage();
    ~tst_QNdefMessage();

private slots:
    void tst_parse_data();
    void tst_parse();
};

tst_QNdefMessage::tst_QNdefMessage()
{
}

tst_QNdefMessage::~tst_QNdefMessage()
{
}

void tst_QNdefMessage::tst_parse_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QNdefMessage>("message");
    QTest::addColumn<QVariantList>("expectedData");

    // empty message
    {
        QByteArray data;
        data.append(char(0xc0));   // MB=1, ME=1
        data.append(char(0));      // TYPE LENGTH
        data.append(char(0));      // PAYLOAD LENGTH 3
        data.append(char(0));      // PAYLOAD LENGTH 2
        data.append(char(0));      // PAYLOAD LENGTH 1
        data.append(char(0));      // PAYLOAD LENGTH 0
        QTest::newRow("empty") << data << QNdefMessage() << QVariantList();

        QNdefRecord record;
        record.setTypeNameFormat(QNdefRecord::Empty);
        QTest::newRow("empty record") << data
                                      << QNdefMessage(record)
                                      << QVariantList();
    }

    // empty short message
    {
        QByteArray data;
        data.append(char(0xd0));   // MB=1, ME=1, SR=1
        data.append(char(0));      // TYPE LENGTH
        data.append(char(0));      // PAYLOAD LENGTH
        QTest::newRow("empty") << data << QNdefMessage() << QVariantList();

        QNdefRecord record;
        record.setTypeNameFormat(QNdefRecord::Empty);
        QTest::newRow("empty record") << data
                                      << QNdefMessage(record)
                                      << QVariantList();
    }

    // unknown message
    {
        QByteArray type("type");
        QByteArray id("id");
        QByteArray payload("payload");

        QByteArray data;
        data.append(char(0xcd));                            // MB=1, ME=1, IL=1, TNF=5
        data.append(char(type.length()));                   // TYPE LENGTH
        data.append(char((payload.length() >> 24) & 0xff)); // PAYLOAD LENGTH 3
        data.append(char((payload.length() >> 16) & 0xff)); // PAYLOAD LENGTH 2
        data.append(char((payload.length() >> 8) & 0xff));  // PAYLOAD LENGTH 1
        data.append(char((payload.length() >> 0) & 0xff));  // PAYLOAD LENGTH 0
        data.append(char(id.length()));                     // ID LENGTH
        data.append(type);
        data.append(id);
        data.append(payload);

        QNdefRecord record;
        record.setTypeNameFormat(QNdefRecord::Unknown);
        record.setType(type);
        record.setId(id);
        record.setPayload(payload);
        QList<QNdefRecord> recordList;
        recordList.append(record);
        QTest::newRow("unknown") << data << QNdefMessage(recordList) << QVariantList();
    }

    // chunked message
    {
        QByteArray type("type");
        QByteArray id("id");
        QByteArray payload("payload");

        QByteArray data;
        data.append(char(0xbd));            // MB=1, CF=1, SR=1, IL=1, TNF=5
        data.append(char(type.length()));   // TYPE LENGTH
        data.append(char(1));               // PAYLOAD LENGTH
        data.append(char(id.length()));     // ID LENGTH
        data.append(type);                  // TYPE
        data.append(id);                    // ID
        data.append(payload.at(0));         // PAYLOAD[0]

        for (int i = 1; i < payload.length() - 1; ++i) {
            data.append(char(0x36));            // CF=1, SR=1, TNF=6
            data.append(char(0));               // TYPE LENGTH
            data.append(char(1));               // PAYLOAD LENGTH
            data.append(payload.at(i));         // PAYLOAD[i]
        }

        data.append(char(0x56));                        // ME=1, SR=1, TNF=6
        data.append(char(0));                           // TYPE LENGTH
        data.append(char(1));                           // PAYLOAD LENGTH
        data.append(payload.at(payload.length() - 1));  // PAYLOAD[length - 1]

        QNdefRecord record;
        record.setTypeNameFormat(QNdefRecord::Unknown);
        record.setType(type);
        record.setId(id);
        record.setPayload(payload);
        QList<QNdefRecord> recordList;
        recordList.append(record);
        QTest::newRow("chunked") << data << QNdefMessage(recordList) << QVariantList();
    }

    // NFC-RTD Text
    {
        QByteArray type("T");
        QByteArray payload;
        payload.append(char(0x02));     // UTF-8, 2 byte language code
        payload.append("en");
        payload.append("Test String");

        QByteArray data;
        data.append(char(0xc1));            // MB=1, ME=1, IL=0, TNF=1
        data.append(char(type.length()));   // TYPE LENGTH
        data.append(char((payload.length() >> 24) & 0xff)); // PAYLOAD LENGTH 3
        data.append(char((payload.length() >> 16) & 0xff)); // PAYLOAD LENGTH 2
        data.append(char((payload.length() >> 8) & 0xff));  // PAYLOAD LENGTH 1
        data.append(char((payload.length() >> 0) & 0xff));  // PAYLOAD LENGTH 0
        data.append(type);
        data.append(payload);

        QNdefRecord record;
        record.setTypeNameFormat(QNdefRecord::NfcRtd);
        record.setType("T");
        record.setPayload("\002enTest String");
        QList<QNdefRecord> recordList;
        recordList.append(record);
        QTest::newRow("nfc-rtd text") << data << QNdefMessage(recordList)
                                      << (QVariantList() << QLatin1String("Test String")
                                                         << QLatin1String("en"));
    }

    // NFC-RTD Text
    {
        QByteArray type("T");
        QByteArray payload;
        payload.append(char(0x02));     // UTF-8, 2 byte language code
        payload.append("ja");
        payload.append(QByteArray::fromHex("e38386e382b9e38388e69687e5ad97e58897"));

        QByteArray data;
        data.append(char(0xc1));            // MB=1, ME=1, IL=0, TNF=1
        data.append(char(type.length()));   // TYPE LENGTH
        data.append(char((payload.length() >> 24) & 0xff)); // PAYLOAD LENGTH 3
        data.append(char((payload.length() >> 16) & 0xff)); // PAYLOAD LENGTH 2
        data.append(char((payload.length() >> 8) & 0xff));  // PAYLOAD LENGTH 1
        data.append(char((payload.length() >> 0) & 0xff));  // PAYLOAD LENGTH 0
        data.append(type);
        data.append(payload);

        QNdefRecord record;
        record.setTypeNameFormat(QNdefRecord::NfcRtd);
        record.setType("T");
        record.setPayload("\002ja" + QByteArray::fromHex("e38386e382b9e38388e69687e5ad97e58897"));
        QList<QNdefRecord> recordList;
        recordList.append(record);
        QTest::newRow("nfc-rtd text ja")
            << data << QNdefMessage(recordList)
            << (QVariantList() << QString::fromUtf8("\343\203\206\343\202\271\343\203\210\346\226"
                                                    "\207\345\255\227\345\210\227")
                               << QLatin1String("ja"));
    }

    // NFC-RTD URI
    {
        QByteArray type("U");
        QByteArray payload;
        payload.append(char(0x00));
        payload.append("http://qt.nokia.com/");

        QByteArray data;
        data.append(char(0xc1));
        data.append(char(type.length()));
        data.append(char((payload.length() >> 24) & 0xff)); // PAYLOAD LENGTH 3
        data.append(char((payload.length() >> 16) & 0xff)); // PAYLOAD LENGTH 2
        data.append(char((payload.length() >> 8) & 0xff));  // PAYLOAD LENGTH 1
        data.append(char((payload.length() >> 0) & 0xff));  // PAYLOAD LENGTH 0
        data.append(type);
        data.append(payload);

        QNdefRecord record;
        record.setTypeNameFormat(QNdefRecord::NfcRtd);
        record.setType("U");
        record.setPayload(QByteArray("\000http://qt.nokia.com/", 21));
        QList<QNdefRecord> recordList;
        recordList.append(record);
        QTest::newRow("nfc-rtd uri http://qt.nokia.com/")
            << data << QNdefMessage(recordList)
            << (QVariantList() << QUrl(QLatin1String("http://qt.nokia.com/")));
    }

    // NFC-RTD URI
    {
        QByteArray type("U");
        QByteArray payload;
        payload.append(char(0x03));
        payload.append("qt.nokia.com/");

        QByteArray data;
        data.append(char(0xc1));
        data.append(char(type.length()));
        data.append(char((payload.length() >> 24) & 0xff)); // PAYLOAD LENGTH 3
        data.append(char((payload.length() >> 16) & 0xff)); // PAYLOAD LENGTH 2
        data.append(char((payload.length() >> 8) & 0xff));  // PAYLOAD LENGTH 1
        data.append(char((payload.length() >> 0) & 0xff));  // PAYLOAD LENGTH 0
        data.append(type);
        data.append(payload);

        QNdefRecord record;
        record.setTypeNameFormat(QNdefRecord::NfcRtd);
        record.setType("U");
        record.setPayload(QByteArray("\003qt.nokia.com/", 14));
        QList<QNdefRecord> recordList;
        recordList.append(record);
        QTest::newRow("nfc-rtd uri abbrev http://qt.nokia.com/")
            << data << QNdefMessage(recordList)
            << (QVariantList() << QUrl(QLatin1String("http://qt.nokia.com/")));
    }

    // NFC-RTD URI
    {
        QByteArray type("U");
        QByteArray payload;
        payload.append(char(0x05));
        payload.append("+1234567890");

        QByteArray data;
        data.append(char(0xc1));
        data.append(char(type.length()));
        data.append(char((payload.length() >> 24) & 0xff)); // PAYLOAD LENGTH 3
        data.append(char((payload.length() >> 16) & 0xff)); // PAYLOAD LENGTH 2
        data.append(char((payload.length() >> 8) & 0xff));  // PAYLOAD LENGTH 1
        data.append(char((payload.length() >> 0) & 0xff));  // PAYLOAD LENGTH 0
        data.append(type);
        data.append(payload);

        QNdefRecord record;
        record.setTypeNameFormat(QNdefRecord::NfcRtd);
        record.setType("U");
        record.setPayload(QByteArray("\005+1234567890", 12));
        QList<QNdefRecord> recordList;
        recordList.append(record);
        QTest::newRow("nfc-rtd uri tel:+1234567890")
            << data << QNdefMessage(recordList)
            << (QVariantList() << QUrl(QLatin1String("tel:+1234567890")));
    }

    // Truncated message
    {
        QByteArray type("U");
        QByteArray id("Test ID");
        QByteArray payload;
        payload.append(char(0x00));
        payload.append("http://qt.nokia.com/");
        QByteArray data;
        data.append(char(0xc9));   // MB=1, ME=1, IL=1

        QTest::newRow("truncated 1") << data << QNdefMessage() << QVariantList();

        data.append(char(type.length()));   // TYPE LENGTH
        QTest::newRow("truncated 2") << data << QNdefMessage() << QVariantList();

        data.append(char((payload.length() >> 24) & 0xff)); // PAYLOAD LENGTH 3
        QTest::newRow("truncated 3") << data << QNdefMessage() << QVariantList();

        data.append(char((payload.length() >> 16) & 0xff)); // PAYLOAD LENGTH 2
        QTest::newRow("truncated 4") << data << QNdefMessage() << QVariantList();

        data.append(char((payload.length() >> 8) & 0xff));  // PAYLOAD LENGTH 1
        QTest::newRow("truncated 5") << data << QNdefMessage() << QVariantList();

        data.append(char((payload.length() >> 0) & 0xff));  // PAYLOAD LENGTH 0
        QTest::newRow("truncated 6") << data << QNdefMessage() << QVariantList();

        data.append(char(id.length())); // ID LENGTH
        QTest::newRow("truncated 7") << data << QNdefMessage() << QVariantList();

        data.append(type);
        QTest::newRow("truncated 8") << data << QNdefMessage() << QVariantList();

        data.append(id);
        QTest::newRow("truncated 9") << data << QNdefMessage() << QVariantList();

        payload.chop(1);
        data.append(payload);
        QTest::newRow("truncated 10") << data << QNdefMessage() << QVariantList();
    }
}

void tst_QNdefMessage::tst_parse()
{
    QFETCH(QByteArray, data);
    QFETCH(QNdefMessage, message);
    QFETCH(QVariantList, expectedData);

    if (QByteArray(QTest::currentDataTag()).startsWith("truncated "))
        QTest::ignoreMessage(QtWarningMsg, "Unexpected end of message");

    QNdefMessage parsedMessage = QNdefMessage::fromByteArray(data);

    QVERIFY(parsedMessage == message);
    QVERIFY(message == parsedMessage);

    QNdefMessage reparsedMessage = QNdefMessage::fromByteArray(message.toByteArray());

    QVERIFY(message == reparsedMessage);
    QVERIFY(reparsedMessage == message);

    for (int i = 0; i < message.count(); ++i) {
        const QNdefRecord &record = message.at(i);
        const QNdefRecord &parsedRecord = parsedMessage.at(i);

        QCOMPARE(record.typeNameFormat(), parsedRecord.typeNameFormat());
        QCOMPARE(record.type(), parsedRecord.type());
        QCOMPARE(record.id(), parsedRecord.id());
        QCOMPARE(record.payload(), parsedRecord.payload());
        QCOMPARE(record.isEmpty(), parsedRecord.isEmpty());

        if (record.isRecordType<QNdefNfcTextRecord>()) {
            QNdefNfcTextRecord textRecord(record);
            QNdefNfcTextRecord parsedTextRecord(parsedRecord);

            QCOMPARE(textRecord.text(), parsedTextRecord.text());
            QCOMPARE(textRecord.locale(), parsedTextRecord.locale());

            if (expectedData.count() == 2) {
                QCOMPARE(parsedTextRecord.text(), expectedData.at(0).toString());
                QCOMPARE(parsedTextRecord.locale(), expectedData.at(1).toString());
            }
        } else if (record.isRecordType<QNdefNfcUriRecord>()) {
            QNdefNfcUriRecord uriRecord(record);
            QNdefNfcUriRecord parsedUriRecord(parsedRecord);

            QCOMPARE(uriRecord.uri(), parsedUriRecord.uri());

            if (expectedData.count() == 1)
                QCOMPARE(parsedUriRecord.uri(), expectedData.at(0).toUrl());
        } else if (record.isRecordType<QNdefRecord>()) {
            QVERIFY(record.isEmpty());
        }
    }
}

QTEST_MAIN(tst_QNdefMessage)

#include "tst_qndefmessage.moc"

