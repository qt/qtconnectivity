/***************************************************************************
**
** Copyright (C) 2014 BlackBerry Limited. All rights reserved.
** Copyright (C) 2014 BasysKom GmbH.
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

#include "neard/neard_helper_p.h"
#include "neard/dbusproperties_p.h"
#include "neard/dbusobjectmanager_p.h"
#include "neard/tag_p.h"

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
        : T(parent),
          m_tagPath(interfacePath),
          m_readRequested(false)
    {
        m_readErrorTimer.setSingleShot(true);
        m_recordPathsCollectedTimer.setSingleShot(true);

        qCDebug(QT_NFC_NEARD) << "tag found at path" << interfacePath.path();
        m_dbusProperties = new OrgFreedesktopDBusPropertiesInterface(QStringLiteral("org.neard"),
                                                                     interfacePath.path(),
                                                                     QDBusConnection::systemBus(),
                                                                     this);
        if (!m_dbusProperties->isValid()) {
            qCWarning(QT_NFC_NEARD) << "Could not connect to dbus property interface at path" << interfacePath.path();
            return;
        }

        QDBusPendingReply<QVariantMap> reply = m_dbusProperties->GetAll(QStringLiteral("org.neard.Tag"));
        reply.waitForFinished();
        if (reply.isError()) {
            qCWarning(QT_NFC_NEARD) << "Could not get properties of org.neard.Tag dbus interface";
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

        qCDebug(QT_NFC_NEARD) << "tag type" << type;

        QObject::connect(&m_recordPathsCollectedTimer, &QTimer::timeout,
                         this,                         &NearFieldTarget::createNdefMessage);
        QObject::connect(&m_readErrorTimer, &QTimer::timeout,
                         this,              &NearFieldTarget::handleReadError);
        QObject::connect(NeardHelper::instance(), &NeardHelper::recordFound,
                         this,                    &NearFieldTarget::handleRecordFound);
    }

    ~NearFieldTarget()
    {
    }

    bool isValid()
    {
        return m_dbusProperties->isValid() && NeardHelper::instance()->dbusObjectManager()->isValid();
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
        return QNearFieldTarget::NdefAccess;
    }

    bool hasNdefMessage()
    {
        return !m_recordPaths.isEmpty();
    }

    QNearFieldTarget::RequestId readNdefMessages()
    {
        if (isValid()) {
            // if the user calls readNdefMessages before the previous request has been completed
            // return the current request id.
            if (m_currentRequestId.isValid())
                return m_currentRequestId;

            QNearFieldTarget::RequestId requestId = QNearFieldTarget::RequestId(new QNearFieldTarget::RequestIdPrivate());
            // save the id so it can be passed along with requestCompleted
            m_currentRequestId = requestId;
            // since the triggering of interfaceAdded will ultimately lead to createNdefMessage being called
            // we need to make sure that ndefMessagesRead will only be triggered when readNdefMessages has
            // been called before. In case readNdefMessages is called again after that we can directly call
            // call createNdefMessage.
            m_readRequested = true;
            if (hasNdefMessage())
                createNdefMessage();
            else
                m_readErrorTimer.start(1000);

            return requestId;
        } else {
            return QNearFieldTarget::RequestId();
        }
    }

    QNearFieldTarget::RequestId sendCommand(const QByteArray &command)
    {
        Q_UNUSED(command);
        return QNearFieldTarget::RequestId();
    }

    QNearFieldTarget::RequestId sendCommands(const QList<QByteArray> &commands)
    {
        Q_UNUSED(commands);
        return QNearFieldTarget::RequestId();
    }

    QNearFieldTarget::RequestId writeNdefMessages(const QList<QNdefMessage> &messages)
    {
        qCDebug(QT_NFC_NEARD) << "writing messages";
        if (messages.isEmpty() || messages.first().isEmpty()) {
            qCWarning(QT_NFC_NEARD) << "No record specified";
            return QNearFieldTarget::RequestId();
        }
        if (messages.count() > 1 || messages.first().count() > 1) {
            // neard only supports one ndef record per tag
            qCWarning(QT_NFC_NEARD) << "Writing of only one NDEF record and message is supported";
            return QNearFieldTarget::RequestId();
        }
        QNdefRecord record = messages.first().first();

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
                return QNearFieldTarget::RequestId();
            }

            OrgNeardTagInterface tagInterface(QStringLiteral("org.neard"),
                                              m_tagPath.path(),
                                              QDBusConnection::systemBus());
            if (!tagInterface.isValid()) {
                qCWarning(QT_NFC_NEARD) << "tag interface invalid";
            } else {
                QDBusPendingReply<> reply;
                reply = tagInterface.Write(recordProperties);
                reply.waitForFinished();
                if (reply.isError()) {
                    qCWarning(QT_NFC_NEARD) << "Error writing to NFC tag" << reply.error();
                    return QNearFieldTarget::RequestId();
                }

                QMetaObject::invokeMethod(this, "ndefMessagesWritten", Qt::QueuedConnection);

                QNearFieldTarget::RequestId requestId = QNearFieldTarget::RequestId(new QNearFieldTarget::RequestIdPrivate());
                QMetaObject::invokeMethod(this, "requestCompleted", Qt::QueuedConnection,
                                          Q_ARG(const QNearFieldTarget::RequestId, requestId));
                return requestId;
            }
        }

        return QNearFieldTarget::RequestId();
    }

private:
    QNdefRecord readRecord(const QDBusObjectPath &path)
    {
        qCDebug(QT_NFC_NEARD) << "reading record for path" << path.path();
        OrgFreedesktopDBusPropertiesInterface recordInterface(QStringLiteral("org.neard"),
                                                              path.path(),
                                                              QDBusConnection::systemBus());
        if (!recordInterface.isValid())
            return QNdefRecord();

        QDBusPendingReply<QVariantMap> reply = recordInterface.GetAll(QStringLiteral("org.neard.Record"));
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

    void handleRecordFound(const QDBusObjectPath &path)
    {
        m_recordPaths.append(path);
        // FIXME: this timer only exists because neard doesn't currently supply enough
        // information to let us know when all record interfaces have been added or
        // how many records are actually contained on a tag. We assume that when no
        // signal has been received for 100ms all record interfaces have been added.
        m_recordPathsCollectedTimer.start(100);
        // as soon as record paths have been added we can handle errors without the timer.
        m_readErrorTimer.stop();
    }

    void createNdefMessage()
    {
        if (m_readRequested) {
            qCDebug(QT_NFC_NEARD) << "creating Ndef message, reading" << m_recordPaths.length() << "record paths";
            QNdefMessage newNdefMessage;
            foreach (const QDBusObjectPath &recordPath, m_recordPaths)
                newNdefMessage.append(readRecord(recordPath));

            if (!newNdefMessage.isEmpty()) {
                QMetaObject::invokeMethod(this, "ndefMessageRead", Qt::QueuedConnection,
                                          Q_ARG(const QNdefMessage, newNdefMessage));
                // the request id in requestCompleted has to match the one created in readNdefMessages
                QMetaObject::invokeMethod(this, "requestCompleted", Qt::QueuedConnection,
                                        Q_ARG(const QNearFieldTarget::RequestId, m_currentRequestId));
            } else {
                QMetaObject::invokeMethod(this, "error", Qt::QueuedConnection,
                                          Q_ARG(const QNearFieldTarget::Error, QNearFieldTarget::UnknownError),
                                          Q_ARG(const QNearFieldTarget::RequestId, m_currentRequestId));
            }

            m_readRequested = false;
            // invalidate the current request id
            m_currentRequestId = QNearFieldTarget::RequestId(0);
        }
    }

    void handleReadError()
    {
        emit QNearFieldTarget::error(QNearFieldTarget::UnknownError, m_currentRequestId);
        m_currentRequestId = QNearFieldTarget::RequestId(0);
    }

protected:
    QDBusObjectPath m_tagPath;
    OrgFreedesktopDBusPropertiesInterface *m_dbusProperties;
    QList<QDBusObjectPath> m_recordPaths;
    QTimer m_recordPathsCollectedTimer;
    QTimer m_readErrorTimer;
    QNearFieldTarget::Type m_type;
    bool m_readRequested;
    QNearFieldTarget::RequestId m_currentRequestId;
};

QT_END_NAMESPACE

#endif // QNEARFIELDTARGET_NEARD_P_H
