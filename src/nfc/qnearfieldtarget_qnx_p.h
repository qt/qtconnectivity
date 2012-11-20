/***************************************************************************
**
** Copyright (C) 2012 Research In Motion
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

#ifndef QNEARFIELDTARGET_QNX_H
#define QNEARFIELDTARGET_QNX_H

#include <qnearfieldtarget.h>
#include <qnearfieldtarget_p.h>
#include <qndefmessage.h>

#include <nfc/nfc.h>
#include <nfc/nfc_types.h>

#include "qnx/qnxnfcmanager_p.h"

QTNFC_BEGIN_NAMESPACE

template <typename T>
class NearFieldTarget : public T
{
public:

    NearFieldTarget(QObject *parent, nfc_target_t *target, QList<QNdefMessage> messages)
        :   QNearFieldTarget(parent), m_target(target)
    {
        char buf[128];
        size_t bufLength;

        if (nfc_get_tag_name(target, buf, 128, &bufLength) != NFC_RESULT_SUCCESS)
            qWarning() << Q_FUNC_INFO << "Could not get tag name";
        else
            m_tagName = QByteArray(buf, bufLength);

        if (nfc_get_tag_id(target, reinterpret_cast<uchar_t *>(buf), 128, &bufLength) != NFC_RESULT_SUCCESS)
            qWarning() << Q_FUNC_INFO << "Could not get tag id";
        else
            m_tagId = QByteArray(buf,bufLength);

        ndefMessages = messages;
    }

    ~NearFieldTarget()
    {
        //Not entierely sure if this is the right place to do that
        nfc_destroy_target(m_target);
        //delete m_target;
    }

    QByteArray uid() const
    {
        //TODO id and not the name should be returned
        return m_tagName;//m_tagId;
    }

    QNearFieldTarget::Type type() const
    {
        //TODO add tag type detection
        return QNearFieldTarget::ProprietaryTag;
    }

    QNearFieldTarget::AccessMethods accessMethods() const
    {
        QNearFieldTarget::AccessMethods result = QNearFieldTarget::NdefAccess;
        return result;
    }

    bool hasNdefMessage()
    {
        return ndefMessages.count() > 0 ? true : false;
    }

    QNearFieldTarget::RequestId readNdefMessages()
    {
        for (int i=0; i<ndefMessages.size(); i++) {
            emit QNearFieldTarget::ndefMessageRead(ndefMessages.at(i));
        }
        return QNearFieldTarget::RequestId();
    }

    QNearFieldTarget::RequestId sendCommand(const QByteArray &command)
    {
        Q_UNUSED(command);
    #if 0
        //Not tested
        bool isSupported;
        nfc_tag_supports_tag_type (m_target,TAG_TYPE_ISO_14443_3, &isSupported);
        nfc_tag_type_t tagType= TAG_TYPE_ISO_14443_3;
        if (!isSupported) {
            nfc_tag_supports_tag_type (m_target,TAG_TYPE_ISO_14443_4, &isSupported);
            tagType = TAG_TYPE_ISO_14443_4;
            if (!isSupported) {
                nfc_tag_supports_tag_type (m_target,TAG_TYPE_ISO_15693_3, &isSupported);
                tagType = TAG_TYPE_ISO_15693_3;
                //We dont support this tag
                if (!isSupported)
                    return QNearFieldTarget::RequestId();
            }
        }
        m_cmdRespons = reinterpret_cast<char *> malloc (256);
        nfc_tag_transceive (m_target, tagType, command.data(), command.length(), m_cmdRespons, 256, &m_cmdResponseLength);
    #else
        return QNearFieldTarget::RequestId();
    #endif
    }

    QNearFieldTarget::RequestId sendCommands(const QList<QByteArray> &commands)
    {
        Q_UNUSED(commands);
        for (int i = 0; i < commands.size(); i++) {
            sendCommand(commands.at(i));
        }
        return QNearFieldTarget::RequestId();
    }

    QNearFieldTarget::RequestId writeNdefMessages(const QList<QNdefMessage> &messages)
    {
        for (int i=0; i<messages.count(); i++) {
            nfc_ndef_message_t *newMessage;
            QByteArray msg = messages.at(i).toByteArray();
            nfc_result_t result = nfc_create_ndef_message_from_bytes(reinterpret_cast<const uchar_t *> (msg.constData()), msg.length(), &newMessage);
            if (result != NFC_RESULT_SUCCESS) {
                qWarning() << Q_FUNC_INFO << "Could not convert QNdefMessage to byte array" << result;
                nfc_delete_ndef_message(newMessage, true);
                return QNearFieldTarget::RequestId();
            }

            result = nfc_write_ndef_message_to_tag(m_target, newMessage, i == 0 ? false : true);
            nfc_delete_ndef_message(newMessage, true);

            if (result != NFC_RESULT_SUCCESS) {
                qWarning() << Q_FUNC_INFO << "Could not write message";
                return QNearFieldTarget::RequestId();
            }

        }
        return QNearFieldTarget::RequestId(new QNearFieldTarget::RequestIdPrivate());
    }

    //Would only be needed for an LLCP socket. But for now there won't any "forum device detected" events
//    nfc_target_t *getTarget()
//    {
//        return m_target;
//    }

protected:
    nfc_target_t *m_target;
#if 0
    char m_cmdRespons;
    size_t m_cmdResponseLength;
#endif
    QByteArray m_tagName;
    QByteArray m_tagId;
    QList<QNdefMessage> ndefMessages;
};

QTNFC_END_NAMESPACE

#endif // QNEARFIELDTARGET_QNX_H
