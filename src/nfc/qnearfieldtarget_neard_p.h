/***************************************************************************
**
** Copyright (C) 2014 BlackBerry Limited. All rights reserved.
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtNfc module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QNEARFIELDTARGET_NEARD_P_H
#define QNEARFIELDTARGET_NEARD_P_H

#include <QDBusObjectPath>
#include <QDBusVariant>

#include <qnearfieldtarget.h>
#include <qnearfieldtarget_p.h>
#include <qndefrecord.h>
#include <qndefmessage.h>

#include "neard/tag_p.h"

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_NFC_NEARD)

template <typename T>
class NearFieldTarget : public T
{
public:

    NearFieldTarget(QObject *parent, QDBusObjectPath interfacePath)
        :   T(parent)
    {
        m_tagInterface = new OrgNeardTagInterface(QStringLiteral("org.neard"),
                                                  interfacePath.path(),
                                                  QDBusConnection::systemBus());

        if (!isValid())
            return;

        QVariantMap properties = m_tagInterface->GetProperties();
        QString type = properties.value(QStringLiteral("Type")).toString();
        m_type = QNearFieldTarget::ProprietaryTag;

        if (type == QStringLiteral("Type 1"))
            m_type = QNearFieldTarget::NfcTagType1;
        else if (type == QStringLiteral("Type 2"))
            m_type = QNearFieldTarget::NfcTagType2;
        else if (type == QStringLiteral("Type 3"))
            m_type = QNearFieldTarget::NfcTagType3;
        else if (type == QStringLiteral("Type 4"))
            m_type = QNearFieldTarget::NfcTagType4;

        qCDebug(QT_NFC_NEARD) << "Type" << type << m_type;
    }

    ~NearFieldTarget()
    {
        delete m_tagInterface;
    }

    bool isValid()
    {
        return m_tagInterface->isValid();
    }

    QByteArray uid() const
    {
        return QByteArray(); // TODO figure out a workaround because neard does not offer
                             // this property
    }

    QNearFieldTarget::Type type() const
    {
        return m_type;
    }

    QNearFieldTarget::AccessMethods accessMethods() const
    {
        QNearFieldTarget::AccessMethods result = QNearFieldTarget::NdefAccess;
        return result;
    }

    bool hasNdefMessage()
    {
        return false;
    }

    QNearFieldTarget::RequestId readNdefMessages()
    {
        if (isValid()) {
            QDBusPendingReply<QVariantMap> reply = m_tagInterface->GetProperties();
            reply.waitForFinished();
            if (reply.isError()) {
                emit QNearFieldTarget::error(QNearFieldTarget::UnknownError, QNearFieldTarget::RequestId());
                return QNearFieldTarget::RequestId();
            }

            const QDBusArgument &recordPaths = qvariant_cast<QDBusArgument>(reply.value().value(QStringLiteral("Records")));

            QNdefMessage newNdefMessage;
            recordPaths.beginArray();
            while (!recordPaths.atEnd()) {
                QDBusObjectPath path;
                recordPaths >> path;
                newNdefMessage.append(readRecord(path));
            }
            recordPaths.endArray();

            emit QNearFieldTarget::ndefMessageRead(newNdefMessage);

            QNearFieldTarget::RequestId requestId = QNearFieldTarget::RequestId(new QNearFieldTarget::RequestIdPrivate());
            QMetaObject::invokeMethod(this, "requestCompleted", Qt::QueuedConnection,
                                      Q_ARG(const QNearFieldTarget::RequestId, requestId));
            return requestId;
        } else {
            emit QNearFieldTarget::error(QNearFieldTarget::UnsupportedError, QNearFieldTarget::RequestId());
            return QNearFieldTarget::RequestId();
        }
    }

    QNearFieldTarget::RequestId sendCommand(const QByteArray &command)
    {
        Q_UNUSED(command);
        emit QNearFieldTarget::error(QNearFieldTarget::UnsupportedError, QNearFieldTarget::RequestId());
        return QNearFieldTarget::RequestId();
    }

    QNearFieldTarget::RequestId sendCommands(const QList<QByteArray> &commands)
    {
        Q_UNUSED(commands);
        emit QNearFieldTarget::error(QNearFieldTarget::UnsupportedError, QNearFieldTarget::RequestId());
        return QNearFieldTarget::RequestId();
    }

    QNearFieldTarget::RequestId writeNdefMessages(const QList<QNdefMessage> &messages)
    {
        Q_UNUSED(messages);
        emit QNearFieldTarget::error(QNearFieldTarget::UnsupportedError, QNearFieldTarget::RequestId());
        return QNearFieldTarget::RequestId();
    }

private:
    QNdefRecord readRecord(QDBusObjectPath path)
    {
        Q_UNUSED(path);
        // TODO
        return QNdefRecord();
    }

protected:
    OrgNeardTagInterface *m_tagInterface;
    QNearFieldTarget::Type m_type;
};

QT_END_NAMESPACE

#endif // QNEARFIELDTARGET_NEARD_P_H
