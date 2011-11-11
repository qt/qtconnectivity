/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
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

#include "../qconnectivitytestcommon.h"

#include <private/qnearfieldmanager_emulator_p.h>
#include <qnearfieldmanager.h>
#include <qndefmessage.h>
#include <qnearfieldtagtype2.h>
#include <qndefnfctextrecord.h>

QTNFC_USE_NAMESPACE

Q_DECLARE_METATYPE(QNearFieldTarget*)

static const char * const deadbeef = "\xde\xad\xbe\xef";

class tst_QNearFieldTagType2 : public QObject
{
    Q_OBJECT

public:
    tst_QNearFieldTagType2();

private slots:
    void init();
    void cleanup();

    void staticMemoryModel();
    void dynamicMemoryModel();

    void ndefMessages();

private:
    void waitForMatchingTarget();

    QNearFieldManagerPrivateImpl *emulatorBackend;
    QNearFieldManager *manager;
    QNearFieldTagType2 *target;
};

tst_QNearFieldTagType2::tst_QNearFieldTagType2()
:   emulatorBackend(0), manager(0), target(0)
{
#ifndef Q_OS_SYMBIAN
    QDir::setCurrent(QLatin1String(SRCDIR));
#endif

    qRegisterMetaType<QNdefMessage>("QNdefMessage");
    qRegisterMetaType<QNearFieldTarget *>("QNearFieldTarget*");
}

void tst_QNearFieldTagType2::init()
{
    emulatorBackend = new QNearFieldManagerPrivateImpl;
    manager = new QNearFieldManager(emulatorBackend, 0);
}

void tst_QNearFieldTagType2::cleanup()
{
    emulatorBackend->reset();

    delete manager;
    manager = 0;
    emulatorBackend = 0;
    target = 0;
}

void tst_QNearFieldTagType2::waitForMatchingTarget()
{
    QSignalSpy targetDetectedSpy(manager, SIGNAL(targetDetected(QNearFieldTarget*)));

    manager->startTargetDetection(QNearFieldTarget::NfcTagType2);

    QTRY_VERIFY(!targetDetectedSpy.isEmpty());

    target = qobject_cast<QNearFieldTagType2 *>(targetDetectedSpy.first().at(0).value<QNearFieldTarget *>());

    manager->stopTargetDetection();

    QVERIFY(target);

    QCOMPARE(target->type(), QNearFieldTarget::NfcTagType2);
}

void tst_QNearFieldTagType2::staticMemoryModel()
{
    waitForMatchingTarget();

    QVERIFY(target->accessMethods() & QNearFieldTarget::TagTypeSpecificAccess);

    QCOMPARE(target->version(), quint8(0x10));

    // readBlock(), writeBlock()
    {
        for (int i = 0; i < 2; ++i) {
            QNearFieldTarget::RequestId id = target->readBlock(i);
            QVERIFY(target->waitForRequestCompleted(id));

            const QByteArray block = target->requestResponse(id).toByteArray();

            id = target->writeBlock(i, QByteArray(4, 0x55));
            QVERIFY(target->waitForRequestCompleted(id));
            QVERIFY(!target->requestResponse(id).toBool());

            id = target->readBlock(i);
            QVERIFY(target->waitForRequestCompleted(id));

            const QByteArray readBlock = target->requestResponse(id).toByteArray();
            QCOMPARE(readBlock, block);
        }

        for (int i = 3; i < 16; ++i) {
            // Read initial data
            QNearFieldTarget::RequestId id = target->readBlock(i);
            QVERIFY(target->waitForRequestCompleted(id));
            QByteArray initialBlock = target->requestResponse(id).toByteArray();

            // Write 0x55
            id = target->writeBlock(i, QByteArray(4, 0x55));
            QVERIFY(target->waitForRequestCompleted(id));
            QVERIFY(target->requestResponse(id).toBool());

            id = target->readBlock(i);
            QVERIFY(target->waitForRequestCompleted(id));
            QByteArray readBlock = target->requestResponse(id).toByteArray();
            QCOMPARE(readBlock, QByteArray(4, 0x55) + initialBlock.mid(4));

            // Write 0xaa
            id = target->writeBlock(i, QByteArray(4, 0xaa));
            QVERIFY(target->waitForRequestCompleted(id));
            QVERIFY(target->requestResponse(id).toBool());

            id = target->readBlock(i);
            QVERIFY(target->waitForRequestCompleted(id));
            readBlock = target->requestResponse(id).toByteArray();
            QCOMPARE(readBlock, QByteArray(4, 0xaa) + initialBlock.mid(4));
        }
    }
}

void tst_QNearFieldTagType2::dynamicMemoryModel()
{
    bool testedStatic = false;
    bool testedDynamic = false;

    QList<QByteArray> seenIds;
    forever {
        waitForMatchingTarget();

        QVERIFY(target->accessMethods() & QNearFieldTarget::TagTypeSpecificAccess);

        QNearFieldTarget::RequestId id = target->readBlock(0);
        QVERIFY(target->waitForRequestCompleted(id));

        const QByteArray data = target->requestResponse(id).toByteArray();
        const QByteArray uid = data.left(3) + data.mid(4, 4);

        if (seenIds.contains(uid))
            break;
        else
            seenIds.append(uid);

        QCOMPARE(target->version(), quint8(0x10));

        bool dynamic = target->memorySize() > 1024;

        if (dynamic) {
            testedDynamic = true;

            int totalBlocks = target->memorySize() / 4;
            int sector1Blocks = qMin(totalBlocks - 256, 256);

            // default sector is sector 0
            for (int i = 3; i < 256; ++i) {
                // Write 0x55
                QNearFieldTarget::RequestId id = target->writeBlock(i, deadbeef);
                QVERIFY(target->waitForRequestCompleted(id));
                QVERIFY(target->requestResponse(id).toBool());
            }

            // change to sector 1
            {
                QNearFieldTarget::RequestId id = target->selectSector(1);
                QVERIFY(target->waitForRequestCompleted(id));
                QVERIFY(target->requestResponse(id).toBool());
            }

            for (int i = 0; i < sector1Blocks; ++i) {
                QNearFieldTarget::RequestId id = target->readBlock(i);
                QVERIFY(target->waitForRequestCompleted(id));
                QByteArray initialBlock = target->requestResponse(id).toByteArray();

                QVERIFY(initialBlock.left(4) != deadbeef);

                // Write 0x55
                id = target->writeBlock(i, QByteArray(4, 0x55));
                QVERIFY(target->waitForRequestCompleted(id));
                QVERIFY(target->requestResponse(id).toBool());

                id = target->readBlock(i);
                QVERIFY(target->waitForRequestCompleted(id));
                QByteArray readBlock = target->requestResponse(id).toByteArray();
                QCOMPARE(readBlock, QByteArray(4, 0x55) + initialBlock.mid(4));

                // Write 0xaa
                id = target->writeBlock(i, QByteArray(4, 0xaa));
                QVERIFY(target->waitForRequestCompleted(id));
                QVERIFY(target->requestResponse(id).toBool());

                id = target->readBlock(i);
                QVERIFY(target->waitForRequestCompleted(id));
                readBlock = target->requestResponse(id).toByteArray();
                QCOMPARE(readBlock, QByteArray(4, 0xaa) + initialBlock.mid(4));
            }

            // change to sector 0
            {
                QNearFieldTarget::RequestId id = target->selectSector(0);
                QVERIFY(target->waitForRequestCompleted(id));
                QVERIFY(target->requestResponse(id).toBool());
            }

            for (int i = 3; i < 256; ++i) {
                QNearFieldTarget::RequestId id = target->readBlock(i);
                QVERIFY(target->waitForRequestCompleted(id));
                QByteArray readBlock = target->requestResponse(id).toByteArray();

                QVERIFY(readBlock.left(4) == deadbeef);
            }
        } else {
            testedStatic = true;

            QNearFieldTarget::RequestId id = target->selectSector(1);
            QVERIFY(target->waitForRequestCompleted(id));
            QVERIFY(!target->requestResponse(id).toBool());
        }
    }

    QVERIFY(testedStatic);
    QVERIFY(testedDynamic);
}

void tst_QNearFieldTagType2::ndefMessages()
{
    QSKIP("Not implemented");

    QByteArray firstId;
    forever {
        waitForMatchingTarget();

        QNearFieldTarget::RequestId id = target->readBlock(0);
        QVERIFY(target->waitForRequestCompleted(id));

        QByteArray uid = target->requestResponse(id).toByteArray().left(3);

        id = target->readBlock(1);
        QVERIFY(target->waitForRequestCompleted(id));
        uid.append(target->requestResponse(id).toByteArray());

        if (firstId.isEmpty())
            firstId = uid;
        else if (firstId == uid)
            break;

        QVERIFY(target->hasNdefMessage());

        QSignalSpy ndefMessageReadSpy(target, SIGNAL(ndefMessageRead(QNdefMessage)));

        target->readNdefMessages();

        QTRY_VERIFY(!ndefMessageReadSpy.isEmpty());

        QList<QNdefMessage> ndefMessages;
        for (int i = 0; i < ndefMessageReadSpy.count(); ++i)
            ndefMessages.append(ndefMessageReadSpy.at(i).first().value<QNdefMessage>());

        QList<QNdefMessage> messages;
        QNdefNfcTextRecord textRecord;
        textRecord.setText(QLatin1String("tst_QNearFieldTagType2::ndefMessages"));

        QNdefMessage message;
        message.append(textRecord);

        if (target->memorySize() > 120) {
            QNdefRecord record;
            record.setTypeNameFormat(QNdefRecord::ExternalRtd);
            record.setType("com.nokia.qt:ndefMessagesTest");
            record.setPayload(QByteArray(120, quint8(0x55)));
            message.append(record);
        }

        messages.append(message);

        QSignalSpy ndefMessageWriteSpy(target, SIGNAL(ndefMessagesWritten()));
        target->writeNdefMessages(messages);

        QTRY_VERIFY(!ndefMessageWriteSpy.isEmpty());

        QVERIFY(target->hasNdefMessage());

        ndefMessageReadSpy.clear();

        target->readNdefMessages();

        QTRY_VERIFY(!ndefMessageReadSpy.isEmpty());

        QList<QNdefMessage> storedMessages;
        for (int i = 0; i < ndefMessageReadSpy.count(); ++i)
            storedMessages.append(ndefMessageReadSpy.at(i).first().value<QNdefMessage>());

        QVERIFY(ndefMessages != storedMessages);

        QVERIFY(messages == storedMessages);
    }
}

QTEST_MAIN(tst_QNearFieldTagType2);

#include "tst_qnearfieldtagtype2.moc"
