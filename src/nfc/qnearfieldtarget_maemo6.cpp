/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
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

#include "qnearfieldtarget_maemo6_p.h"

QTNFC_BEGIN_NAMESPACE

void PendingCallWatcher::addSendCommand(const QDBusPendingReply<QByteArray> &reply,
                                        const QNearFieldTarget::RequestId &id)
{
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
            this, SLOT(sendCommandFinished(QDBusPendingCallWatcher*)));

    m_pendingCommands.insert(watcher, id);
}

void PendingCallWatcher::addReadNdefMessages(const QDBusPendingReply<QList<QByteArray> > &reply,
                                             const QNearFieldTarget::RequestId &id)
{
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
            this, SLOT(readNdefMessagesFinished(QDBusPendingCallWatcher*)));

    m_pendingNdefReads.insert(watcher, id);
}

void PendingCallWatcher::addWriteNdefMessages(const QDBusPendingReply<> &reply,
                                              const QNearFieldTarget::RequestId &id)
{
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
            this, SLOT(writeNdefMessages(QDBusPendingCallWatcher*)));

    m_pendingNdefWrites.insert(watcher, id);
}

void PendingCallWatcher::sendCommandFinished(QDBusPendingCallWatcher *watcher)
{
    QNearFieldTarget::RequestId id = m_pendingCommands.take(watcher);

    if (!id.isValid()) {
        watcher->deleteLater();
        return;
    }

    QDBusPendingReply<QByteArray> reply = *watcher;
    if (reply.isError()) {
        QMetaObject::invokeMethod(parent(), "error",
                                  Q_ARG(QNearFieldTarget::Error, QNearFieldTarget::UnknownError),
                                  Q_ARG(QNearFieldTarget::RequestId, id));
    } else {
        const QByteArray data = reply.argumentAt<0>();
        QMetaObject::invokeMethod(parent(), "handleResponse",
                                  Q_ARG(QNearFieldTarget::RequestId, id),
                                  Q_ARG(QByteArray, data));
    }

    watcher->deleteLater();
}

void PendingCallWatcher::readNdefMessagesFinished(QDBusPendingCallWatcher *watcher)
{
    QNearFieldTarget::RequestId id = m_pendingNdefReads.take(watcher);

    if (!id.isValid()) {
        watcher->deleteLater();
        return;
    }

    QDBusPendingReply<QList<QByteArray> > reply = *watcher;
    if (reply.isError()) {
        QMetaObject::invokeMethod(parent(), "error",
                                  Q_ARG(QNearFieldTarget::Error, QNearFieldTarget::NdefReadError),
                                  Q_ARG(QNearFieldTarget::RequestId, id));
    } else {
        const QList<QByteArray> data = reply.argumentAt<0>();
        foreach (const QByteArray &m, data) {
            qDebug() << Q_FUNC_INFO << m.toHex();
            const QNdefMessage message = QNdefMessage::fromByteArray(m);

            qDebug() << "record count:" << message.count();
            foreach (const QNdefRecord &record, message)
                qDebug() << record.typeNameFormat() << record.type() << record.payload().toHex();

            QMetaObject::invokeMethod(parent(), "ndefMessageRead", Q_ARG(QNdefMessage, message));
        }

        QMetaObject::invokeMethod(parent(), "requestCompleted",
                                  Q_ARG(QNearFieldTarget::RequestId, id));
    }

    watcher->deleteLater();
}

void PendingCallWatcher::writeNdefMessages(QDBusPendingCallWatcher *watcher)
{
    QNearFieldTarget::RequestId id = m_pendingNdefWrites.take(watcher);

    if (!id.isValid()) {
        watcher->deleteLater();
        return;
    }

    QDBusPendingReply<> reply = *watcher;
    if (reply.isError()) {
        QMetaObject::invokeMethod(parent(), "error",
                                  Q_ARG(QNearFieldTarget::Error, QNearFieldTarget::NdefWriteError),
                                  Q_ARG(QNearFieldTarget::RequestId, id));
    } else {
        QMetaObject::invokeMethod(parent(), "ndefMessagesWritten");
        QMetaObject::invokeMethod(parent(), "requestCompleted",
                                  Q_ARG(QNearFieldTarget::RequestId, id));
    }

    watcher->deleteLater();
}

int TagType1::memorySize() const
{
    return m_tag->size();
}

int TagType2::memorySize() const
{
    return m_tag->size();
}

int TagType3::memorySize() const
{
    return m_tag->size();
}

int TagType4::memorySize() const
{
    return m_tag->size();
}

#include <moc_qnearfieldtarget_maemo6_p.cpp>

QTNFC_END_NAMESPACE
