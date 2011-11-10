/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
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

#include "qnearfieldmanager_simulator_p.h"
#include "qnearfieldmanager.h"
#include "qnearfieldtarget_p.h"
#include "qnearfieldtagtype1.h"
#include "qndefmessage.h"

#include <mobilityconnection_p.h>
#include <QtGui/private/qsimulatordata_p.h>

#include <QtCore/QCoreApplication>

QTNFC_BEGIN_NAMESPACE

using namespace QtSimulatorPrivate;

namespace Simulator {

class TagType1 : public QNearFieldTagType1
{
public:
    TagType1(const QByteArray &uid, QObject *parent);
    ~TagType1();

    QByteArray uid() const;

    AccessMethods accessMethods() const;

    RequestId sendCommand(const QByteArray &command);
    bool waitForRequestCompleted(const RequestId &id, int msecs = 5000);

private:
    QByteArray m_uid;
};

TagType1::TagType1(const QByteArray &uid, QObject *parent)
:   QNearFieldTagType1(parent), m_uid(uid)
{
}

TagType1::~TagType1()
{
}

QByteArray TagType1::uid() const
{
    return m_uid;
}

QNearFieldTarget::AccessMethods TagType1::accessMethods() const
{
    return NdefAccess | TagTypeSpecificAccess;
}

QNearFieldTarget::RequestId TagType1::sendCommand(const QByteArray &command)
{
    quint16 crc = qNfcChecksum(command.constData(), command.length());

    RequestId id(new RequestIdPrivate);

    MobilityConnection *connection = MobilityConnection::instance();
    QByteArray response =
        RemoteMetacall<QByteArray>::call(connection->sendSocket(), WaitSync, "nfcSendCommand",
                                         command + char(crc & 0xff) + char(crc >> 8));

    if (response.isEmpty()) {
        QMetaObject::invokeMethod(this, "error", Qt::QueuedConnection,
                                  Q_ARG(QNearFieldTarget::Error, NoResponseError),
                                  Q_ARG(QNearFieldTarget::RequestId, id));
        return id;
    }

    // check crc
    if (qNfcChecksum(response.constData(), response.length()) != 0) {
        QMetaObject::invokeMethod(this, "error", Qt::QueuedConnection,
                                  Q_ARG(QNearFieldTarget::Error, ChecksumMismatchError),
                                  Q_ARG(QNearFieldTarget::RequestId, id));
        return id;
    }

    response.chop(2);

    QMetaObject::invokeMethod(this, "handleResponse", Qt::QueuedConnection,
                              Q_ARG(QNearFieldTarget::RequestId, id), Q_ARG(QByteArray, response));

    return id;
}

bool TagType1::waitForRequestCompleted(const RequestId &id, int msecs)
{
    QCoreApplication::sendPostedEvents(this, QEvent::MetaCall);

    return QNearFieldTagType1::waitForRequestCompleted(id, msecs);
}

class NfcConnection : public QObject
{
    Q_OBJECT

public:
    NfcConnection();
    virtual ~NfcConnection();

signals:
    void targetEnteringProximity(const QByteArray &uid);
    void targetLeavingProximity(const QByteArray &uid);
};

NfcConnection::NfcConnection()
:   QObject(MobilityConnection::instance())
{
    MobilityConnection *connection = MobilityConnection::instance();
    connection->addMessageHandler(this);

    RemoteMetacall<void>::call(connection->sendSocket(), NoSync, "setRequestsNfc");
}

NfcConnection::~NfcConnection()
{
}

}

QNearFieldManagerPrivateImpl::QNearFieldManagerPrivateImpl()
:   nfcConnection(new Simulator::NfcConnection)
{
    connect(nfcConnection, SIGNAL(targetEnteringProximity(QByteArray)),
            this, SLOT(targetEnteringProximity(QByteArray)));
    connect(nfcConnection, SIGNAL(targetLeavingProximity(QByteArray)),
            this, SLOT(targetLeavingProximity(QByteArray)));
}

QNearFieldManagerPrivateImpl::~QNearFieldManagerPrivateImpl()
{
    delete nfcConnection;
}

bool QNearFieldManagerPrivateImpl::isAvailable() const
{
    return true;
}

void QNearFieldManagerPrivateImpl::targetEnteringProximity(const QByteArray &uid)
{
    QNearFieldTarget *target = m_targets.value(uid).data();
    if (!target) {
        target = new Simulator::TagType1(uid, this);
        m_targets.insert(uid, target);
    }

    targetActivated(target);
}

void QNearFieldManagerPrivateImpl::targetLeavingProximity(const QByteArray &uid)
{
    QNearFieldTarget *target = m_targets.value(uid).data();
    if (!target) {
        m_targets.remove(uid);
        return;
    }

    targetDeactivated(target);
}

#include "qnearfieldmanager_simulator.moc"
#include "moc_qnearfieldmanager_simulator_p.cpp"

QTNFC_END_NAMESPACE
