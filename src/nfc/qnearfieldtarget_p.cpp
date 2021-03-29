/****************************************************************************
**
** Copyright (C) 2017 Governikus GmbH & Co. K
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtNfc module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qnearfieldtarget_p.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QElapsedTimer>
#include <QtCore/QPointer>

QT_BEGIN_NAMESPACE


QNearFieldTargetPrivate::QNearFieldTargetPrivate(QObject *parent)
:   QObject(parent)
,   q_ptr(nullptr)
{
}

QByteArray QNearFieldTargetPrivate::uid() const
{
    return QByteArray();
}

QNearFieldTarget::Type QNearFieldTargetPrivate::type() const
{
    return QNearFieldTarget::Type::ProprietaryTag;
}

QNearFieldTarget::AccessMethods QNearFieldTargetPrivate::accessMethods() const
{
    return QNearFieldTarget::AccessMethod::UnknownAccess;
}

bool QNearFieldTargetPrivate::disconnect()
{
    return false;
}

// NdefAccess
bool QNearFieldTargetPrivate::hasNdefMessage()
{
    return false;
}

QNearFieldTarget::RequestId QNearFieldTargetPrivate::readNdefMessages()
{
    const QNearFieldTarget::RequestId id;
    Q_EMIT error(QNearFieldTarget::UnsupportedError, id);
    return id;
}

NearFieldTarget::RequestId QNearFieldTargetPrivate::writeNdefMessages(const QList<QNdefMessage> &messages)
{
    Q_UNUSED(messages);

    const QNearFieldTarget::RequestId id;
    Q_EMIT error(QNearFieldTarget::UnsupportedError, id);
    return id;
}

// TagTypeSpecificAccess
int QNearFieldTargetPrivate::maxCommandLength() const
{
    return 0;
}

NearFieldTarget::RequestId QNearFieldTargetPrivate::sendCommand(const QByteArray &command)
{
    Q_UNUSED(command);

    const QNearFieldTarget::RequestId id;
    Q_EMIT error(QNearFieldTarget::UnsupportedError, id);
    return id;
}

bool QNearFieldTargetPrivate::waitForRequestCompleted(const NearFieldTarget::RequestId &id, int msecs)
{
    QElapsedTimer timer;
    timer.start();

    const QPointer<QNearFieldTargetPrivate> weakThis = this;

    do {
        if (!weakThis)
            return false;

        if (m_decodedResponses.contains(id))
            return true;
        else
            QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents, 1);
    } while (timer.elapsed() <= msecs);

    reportError(QNearFieldTarget::TimeoutError, id);

    return false;
}

QVariant QNearFieldTargetPrivate::requestResponse(const NearFieldTarget::RequestId &id) const
{
    return m_decodedResponses.value(id);
}

void QNearFieldTargetPrivate::setResponseForRequest(const NearFieldTarget::RequestId &id,
                                                    const QVariant &response,
                                                    bool emitRequestCompleted)
{
    for (auto i = m_decodedResponses.begin(), end = m_decodedResponses.end(); i != end; /* erasing */) {
        // no more external references
        if (i.key().refCount() == 1)
            i = m_decodedResponses.erase(i);
        else
            ++i;
    }

    m_decodedResponses.insert(id, response);

    if (emitRequestCompleted)
        Q_EMIT requestCompleted(id);
}

void QNearFieldTargetPrivate::reportError(NearFieldTarget::Error error, const NearFieldTarget::RequestId &id)
{
    setResponseForRequest(id, QVariant(), false);
    QMetaObject::invokeMethod(this, [this, error, id]() {
        Q_EMIT this->error(error, id);
    }, Qt::QueuedConnection);
}

QT_END_NAMESPACE
