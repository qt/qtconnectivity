/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtNfc module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QTest>

#include <QNdefFilter>
#include <QNdefNfcSmartPosterRecord>
#include <QNdefNfcTextRecord>
#include <QNdefNfcUriRecord>

QT_USE_NAMESPACE

class tst_QNdefFilter : public QObject
{
    Q_OBJECT
private slots:
    void construct();

    void appendRecord();
    void appendRecord_data();

    void appendRecordParameters();
    void appendRecordParameters_data();

    void appendRecordTemplate();
    void appendRecordTemplate_data();
};

void tst_QNdefFilter::construct()
{
    QNdefFilter filter;
    QCOMPARE(filter.recordCount(), 0);
    QCOMPARE(filter.orderMatch(), false);
}

void tst_QNdefFilter::appendRecord()
{
    QFETCH(QNdefRecord::TypeNameFormat, typeNameFormat);
    QFETCH(QByteArray, type);
    QFETCH(unsigned int, minimum);
    QFETCH(unsigned int, maximum);
    QFETCH(bool, result);

    QNdefFilter filter;

    const QNdefFilter::Record record { typeNameFormat, type, minimum, maximum };
    QVERIFY(filter.appendRecord(record) == result);
    const int desiredRecordCount = result ? 1 : 0;
    QCOMPARE(filter.recordCount(), desiredRecordCount);
}

void tst_QNdefFilter::appendRecord_data()
{
    QTest::addColumn<QNdefRecord::TypeNameFormat>("typeNameFormat");
    QTest::addColumn<QByteArray>("type");
    QTest::addColumn<unsigned int>("minimum");
    QTest::addColumn<unsigned int>("maximum");
    QTest::addColumn<bool>("result");

    const QMap<QByteArray, QNdefRecord::TypeNameFormat> inputs {
        { "Empty", QNdefRecord::Empty },
        { "NfcRtd", QNdefRecord::NfcRtd },
        { "Mime", QNdefRecord::Mime },
        { "Uri", QNdefRecord::Uri},
        { "ExternalRtd", QNdefRecord::ExternalRtd },
        { "Unknown", QNdefRecord::Unknown }
    };

    for (auto it = inputs.cbegin(); it != inputs.cend(); ++it) {
        const auto type = it.key();
        const auto format = it.value();
        QTest::newRow(type + "; min < max") << format << type << 1u << 2u << true;
        QTest::newRow(type + "; min == max") << format << type << 2u << 2u << true;
        QTest::newRow(type + "; min > max") << format << type << 2u << 1u << false;
    }
}

void tst_QNdefFilter::appendRecordParameters()
{
    QFETCH(QNdefRecord::TypeNameFormat, typeNameFormat);
    QFETCH(QByteArray, type);
    QFETCH(unsigned int, minimum);
    QFETCH(unsigned int, maximum);
    QFETCH(bool, result);

    QNdefFilter filter;

    QVERIFY(filter.appendRecord(typeNameFormat, type, minimum, maximum) == result);
    const int desiredRecordCount = result ? 1 : 0;
    QCOMPARE(filter.recordCount(), desiredRecordCount);
}

void tst_QNdefFilter::appendRecordParameters_data()
{
    appendRecord_data();
}

void tst_QNdefFilter::appendRecordTemplate()
{
    QFETCH(QByteArray, type);
    QFETCH(unsigned int, minimum);
    QFETCH(unsigned int, maximum);
    QFETCH(bool, result);

    QNdefFilter filter;
    if (type == QByteArray("SmartPoster"))
        QVERIFY(filter.appendRecord<QNdefNfcSmartPosterRecord>(minimum, maximum) == result);
    else if (type == QByteArray("Text"))
        QVERIFY(filter.appendRecord<QNdefNfcTextRecord>(minimum, maximum) == result);
    else if (type == QByteArray("Uri"))
        QVERIFY(filter.appendRecord<QNdefNfcUriRecord>(minimum, maximum) == result);

    const int desiredRecordCount = result ? 1 : 0;
    QCOMPARE(filter.recordCount(), desiredRecordCount);
}

void tst_QNdefFilter::appendRecordTemplate_data()
{
    QTest::addColumn<QByteArray>("type");
    QTest::addColumn<unsigned int>("minimum");
    QTest::addColumn<unsigned int>("maximum");
    QTest::addColumn<bool>("result");

    const QList<QByteArray> types {"SmartPoster", "Text", "Uri"};
    for (const auto &type : types) {
        QTest::newRow(type + "; min < max") << type << 1u << 2u << true;
        QTest::newRow(type + "; min == max") << type << 2u << 2u << true;
        QTest::newRow(type + "; min > max") << type << 2u << 1u << false;
    }
}

QTEST_MAIN(tst_QNdefFilter)

#include "tst_qndeffilter.moc"
