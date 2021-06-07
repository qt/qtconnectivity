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

#include <QtTest/QtTest>

#include <qnearfieldmanager_emulator_p.h>
#include <QtNfc/qnearfieldmanager.h>
#include <QtNfc/qndefnfctextrecord.h>
#include <QtNfc/qndefnfcurirecord.h>
#include <QtNfc/qndefmessage.h>
#include <QtNfc/qndefrecord.h>

QT_USE_NAMESPACE

Q_DECLARE_METATYPE(QNearFieldTarget*)
Q_DECLARE_METATYPE(QNearFieldTarget::Type)
Q_DECLARE_METATYPE(QNdefFilter)
Q_DECLARE_METATYPE(QNdefRecord::TypeNameFormat)

class tst_QNearFieldManager : public QObject
{
    Q_OBJECT

public:
    tst_QNearFieldManager();

private slots:
    void initTestCase();

    void isSupported();
    void isSupported_data();

    void userInformation();

    void targetDetected_data();
    void targetDetected();
};

tst_QNearFieldManager::tst_QNearFieldManager()
{
    QDir::setCurrent(QLatin1String(SRCDIR));

    qRegisterMetaType<QNdefMessage>();
    qRegisterMetaType<QNearFieldTarget *>();
}

void tst_QNearFieldManager::initTestCase()
{
    QNearFieldManagerPrivateImpl *emulatorBackend = new QNearFieldManagerPrivateImpl;
    QNearFieldManager manager(emulatorBackend, 0);

    QVERIFY(manager.isEnabled());
}

void tst_QNearFieldManager::isSupported()
{
    QFETCH(QNearFieldTarget::AccessMethod, accessMethod);
    QFETCH(bool, supported);

    QNearFieldManagerPrivateImpl *emulatorBackend = new QNearFieldManagerPrivateImpl;
    QNearFieldManager manager(emulatorBackend, nullptr);

    QCOMPARE(manager.isSupported(accessMethod), supported);
}

void tst_QNearFieldManager::isSupported_data()
{
    QTest::addColumn<QNearFieldTarget::AccessMethod>("accessMethod");
    QTest::addColumn<bool>("supported");

    QTest::newRow("UnknownAccess") << QNearFieldTarget::UnknownAccess << false;
    QTest::newRow("NdefAccess") << QNearFieldTarget::NdefAccess << true;
    QTest::newRow("TagTypeSpecificAccess") << QNearFieldTarget::TagTypeSpecificAccess << false;
    QTest::newRow("AnyAccess") << QNearFieldTarget::AnyAccess << true;
}

void tst_QNearFieldManager::userInformation()
{
    QNearFieldManagerPrivateImpl *emulatorBackend = new QNearFieldManagerPrivateImpl;
    QNearFieldManager manager(emulatorBackend, nullptr);

    QSignalSpy spy(emulatorBackend, &QNearFieldManagerPrivateImpl::userInformationChanged);

    manager.startTargetDetection(QNearFieldTarget::AnyAccess);

    const QString progressString("NFC target detection in progress");
    manager.setUserInformation(progressString);

    const QString errorString("Failed to detect NFC targets");
    manager.stopTargetDetection(errorString);

    QCOMPARE(spy.count(), 2);
    QCOMPARE(spy.at(0).at(0).toString(), progressString);
    QCOMPARE(spy.at(1).at(0).toString(), errorString);
}

void tst_QNearFieldManager::targetDetected_data()
{
    QTest::addColumn<QNearFieldTarget::AccessMethod>("accessMethod");
    QTest::addColumn<bool>("deleteTarget");

    QTest::newRow("UnknownAccess") << QNearFieldTarget::UnknownAccess << false;
    QTest::newRow("NdefAccess, no delete") << QNearFieldTarget::NdefAccess << false;
    QTest::newRow("AnyAccess, delete") << QNearFieldTarget::AnyAccess << true;
}

void tst_QNearFieldManager::targetDetected()
{
    QFETCH(QNearFieldTarget::AccessMethod, accessMethod);
    QFETCH(bool, deleteTarget);

    QNearFieldManagerPrivateImpl *emulatorBackend = new QNearFieldManagerPrivateImpl;
    QNearFieldManager manager(emulatorBackend, nullptr);

    QSignalSpy targetDetectedSpy(&manager, &QNearFieldManager::targetDetected);
    QSignalSpy targetLostSpy(&manager, &QNearFieldManager::targetLost);
    QSignalSpy detectionStoppedSpy(&manager, &QNearFieldManager::targetDetectionStopped);

    const bool started = manager.startTargetDetection(accessMethod);

    if (started) {
        QTRY_VERIFY(!targetDetectedSpy.isEmpty());

        QNearFieldTarget *target = targetDetectedSpy.first().at(0).value<QNearFieldTarget *>();

        QSignalSpy disconnectedSpy(target, SIGNAL(disconnected()));

        QVERIFY(target);

        QVERIFY(!target->uid().isEmpty());

        if (!deleteTarget) {
            QTRY_VERIFY(!targetLostSpy.isEmpty());

            QNearFieldTarget *lostTarget = targetLostSpy.first().at(0).value<QNearFieldTarget *>();

            QCOMPARE(target, lostTarget);

            QVERIFY(!disconnectedSpy.isEmpty());
        } else {
            delete target;

            // wait for another targetDetected() without a targetLost() signal in between.
            targetDetectedSpy.clear();
            targetLostSpy.clear();

            QTRY_VERIFY(targetLostSpy.isEmpty() && !targetDetectedSpy.isEmpty());
        }
    }

    manager.stopTargetDetection();

    QCOMPARE(detectionStoppedSpy.count(), 1);
}

QTEST_MAIN(tst_QNearFieldManager)

// Unset the moc namespace which is not required for the following include.
#undef QT_BEGIN_MOC_NAMESPACE
#define QT_BEGIN_MOC_NAMESPACE
#undef QT_END_MOC_NAMESPACE
#define QT_END_MOC_NAMESPACE

#include "tst_qnearfieldmanager.moc"
