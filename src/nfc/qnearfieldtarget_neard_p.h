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
#include "neard/record_p.h"

#include <qndefnfctextrecord.h>
#include <qndefnfcsmartposterrecord.h>
#include <qndefnfcurirecord.h>

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
                                                  QDBusConnection::systemBus(), this);

        if (!isValid()) {
            qCWarning(QT_NFC_NEARD) << "Could not connect to org.neard.Tag dbus interface"
                       << interfacePath.path();
            return;
        }

        QDBusPendingReply<QVariantMap> reply = m_tagInterface->GetProperties();
        reply.waitForFinished();
        if (reply.isError()) {
            qCWarning(QT_NFC_NEARD) << "Could not get properties of org.neard.Tag dbus interface"
                       << interfacePath.path();
            return;
        }

        const QString &type = reply.value().value(QStringLiteral("Type")).toString();
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
        if (isValid()) {
            QDBusPendingReply<QVariantMap> reply = m_tagInterface->GetProperties();
            reply.waitForFinished();
            if (reply.isError())
                return false;

            const QDBusArgument &recordPaths = qvariant_cast<QDBusArgument>(reply.value().value(QStringLiteral("Records")));

            recordPaths.beginArray();
            while (!recordPaths.atEnd()) {
                QDBusObjectPath path;
                recordPaths >> path;
                if (!path.path().isEmpty()) {
                    recordPaths.endArray();
                    return true;
                }
            }
            recordPaths.endArray();
        }
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
        if (messages.isEmpty() || messages.first().isEmpty()) {
            qCWarning(QT_NFC_NEARD) << "No record specified";
            //TODO
            emit QNearFieldTarget::error(QNearFieldTarget::UnsupportedError, QNearFieldTarget::RequestId());
            return QNearFieldTarget::RequestId();
        }
        if (messages.count() > 1 || messages.first().count() > 1) {
            // neard only supports one ndef record per tag
            qCWarning(QT_NFC_NEARD) << "Writing of only one NDEF record and message is supported";
            emit QNearFieldTarget::error(QNearFieldTarget::UnsupportedError, QNearFieldTarget::RequestId());
            return QNearFieldTarget::RequestId();
        }
        QNdefRecord record = messages.first().first();

        QDBusPendingReply<> reply;
        if (record.typeNameFormat() == QNdefRecord::NfcRtd) {
            QVariantMap recordProperties;
            if (record.isRecordType<QNdefNfcUriRecord>()) {
                recordProperties.insert(QStringLiteral("Type"), QStringLiteral("URI"));
                QNdefNfcUriRecord uriRecord = static_cast<QNdefNfcUriRecord>(record);
                recordProperties.insert(QStringLiteral("URI"), uriRecord.uri().path());
            } else if (record.isRecordType<QNdefNfcSmartPosterRecord>()) {
                recordProperties.insert(QStringLiteral("Type"), QStringLiteral("SmartPoster"));
                QNdefNfcSmartPosterRecord spRecord = static_cast<QNdefNfcSmartPosterRecord>(record);
                recordProperties.insert(QStringLiteral("URI"), spRecord.uri().path());
                // Currently neard only supports the uri property for writing
            } else if (record.isRecordType<QNdefNfcTextRecord>()) {
                recordProperties.insert(QStringLiteral("Type"), QStringLiteral("Text"));
                QNdefNfcTextRecord textRecord = static_cast<QNdefNfcTextRecord>(record);
                recordProperties.insert(QStringLiteral("Representation"), textRecord.text());
                recordProperties.insert(QStringLiteral("Encoding"),
                                        textRecord.encoding() == QNdefNfcTextRecord::Utf8 ?
                                              QStringLiteral("UTF-8") : QStringLiteral("UTF-16") );
                recordProperties.insert(QStringLiteral("Language"), textRecord.locale());
            } else {
                qCWarning(QT_NFC_NEARD) << "Record type not supported for writing";
                emit QNearFieldTarget::error(QNearFieldTarget::UnsupportedError, QNearFieldTarget::RequestId());
                return QNearFieldTarget::RequestId();
            }

            reply = m_tagInterface->Write(recordProperties);
        }

        reply.waitForFinished();
        if (reply.isError()) {
            qCWarning(QT_NFC_NEARD) << "Error writing to NFC tag" << reply.error();
            emit QNearFieldTarget::error(QNearFieldTarget::UnknownError, QNearFieldTarget::RequestId());
            return QNearFieldTarget::RequestId();
        }

        emit QNearFieldTarget::ndefMessagesWritten();

        QNearFieldTarget::RequestId requestId = QNearFieldTarget::RequestId(new QNearFieldTarget::RequestIdPrivate());
        QMetaObject::invokeMethod(this, "requestCompleted", Qt::QueuedConnection,
                                  Q_ARG(const QNearFieldTarget::RequestId, requestId));
        return requestId;
    }

private:
    QNdefRecord readRecord(const QDBusObjectPath &path)
    {
        OrgNeardRecordInterface recordInterface(QStringLiteral("org.neard"),
                                                path.path(),
                                                QDBusConnection::systemBus());
        if (!recordInterface.isValid())
            return QNdefRecord();

        QDBusPendingReply<QVariantMap> reply = recordInterface.GetProperties();
        reply.waitForFinished();
        if (reply.isError())
            return QNdefRecord();

        const QString &value = reply.value().value(QStringLiteral("Representation")).toString();
        const QString &locale = reply.value().value(QStringLiteral("Language")).toString();
        const QString &encoding = reply.value().value(QStringLiteral("Encoding")).toString();
        const QString &uri = reply.value().value(QStringLiteral("URI")).toString();
//        const QString &mimetype = reply.value().value(QStringLiteral("MIMEType")).toString();
//        const QString &mime = reply.value().value(QStringLiteral("MIME")).toString();
//        const QString &arr = reply.value().value(QStringLiteral("ARR")).toString();
//        const QString &size = reply.value().value(QStringLiteral("Size")).toString();

        const QString type = reply.value().value(QStringLiteral("Type")).toString();
        if (type == QStringLiteral("Text")) {
            QNdefNfcTextRecord textRecord;
            textRecord.setText(value);
            textRecord.setLocale(locale);
            textRecord.setEncoding((encoding == QStringLiteral("UTF-8")) ? QNdefNfcTextRecord::Utf8
                                                                       : QNdefNfcTextRecord::Utf16);
            return textRecord;
        } else if (type == QStringLiteral("SmartPoster")) {
            QNdefNfcSmartPosterRecord spRecord;
            if (!value.isEmpty()) {
                spRecord.addTitle(value, locale, (encoding == QStringLiteral("UTF-8"))
                                                        ? QNdefNfcTextRecord::Utf8
                                                        : QNdefNfcTextRecord::Utf16);
            }

            if (!uri.isEmpty())
                spRecord.setUri(QUrl(uri));

//            const QString &actionString = reply.value().value(QStringLiteral("Action")).toString();
//            if (!action.isEmpty()) {
//                QNdefNfcSmartPosterRecord::Action action;

//                spRecord.setAction(acti);
//            }

            return spRecord;
        } else if (type == QStringLiteral("URI")) {
            QNdefNfcUriRecord uriRecord;
            uriRecord.setUri(QUrl(uri));
            return uriRecord;
        } else if (type == QStringLiteral("MIME")) {

        } else if (type == QStringLiteral("AAR")) {

        }

        return QNdefRecord();
    }

protected:
    OrgNeardTagInterface *m_tagInterface;
    QNearFieldTarget::Type m_type;
};

QT_END_NAMESPACE

#endif // QNEARFIELDTARGET_NEARD_P_H
