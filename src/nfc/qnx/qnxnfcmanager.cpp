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

#include "qnxnfcmanager_p.h"
#include <QMetaMethod>
#include <QMetaObject>

QTNFC_BEGIN_NAMESPACE

QNXNFCManager *QNXNFCManager::m_instance = 0;

QNXNFCManager *QNXNFCManager::instance()
{
    if (!m_instance)
        m_instance = new QNXNFCManager;

    return m_instance;
}

void QNXNFCManager::registerForNewInstance()
{
    m_instanceCount++;
}

void QNXNFCManager::unregisterForInstance()
{
    if (m_instanceCount>=1) {
        m_instanceCount--;
        if (m_instanceCount==0) {
            delete m_instance;
            m_instance = 0;
        }
    } else {
        qWarning() << Q_FUNC_INFO << "instance count below 0";
    }
}

nfc_target_t *QNXNFCManager::getLastTarget()
{
    return m_lastTarget;
}

bool QNXNFCManager::isAvailable()
{
    return m_available;
}

QNXNFCManager::QNXNFCManager()
    : QObject(), nfcNotifier(0)
{
    //bb::system::InvokeManager *iManager = new bb::system::InvokeManager(this);
    //connect(iManager, SIGNAL(invoked(const bb::system::InvokeRequest&)),
    //        this, SLOT(handleInvoke(const bb::system::InvokeRequest&)));

    nfc_set_verbosity(2);
    qQNXNFCDebug()<<"Init BB NFC";

    if (nfc_connect() != NFC_RESULT_SUCCESS) {
        qWarning()<<Q_FUNC_INFO<< "Could not connect to NFC System";
        return;
    }

    bool nfcStatus;
    nfc_get_setting(NFC_SETTING_ENABLED, &nfcStatus);
    qQNXNFCDebug() << "NFC status" << nfcStatus;
    if (!nfcStatus) {
        qWarning() << "NFC not enabled...enabling";
        //qDebug()<<nfc_set_setting(NFC_SETTING_ENABLED, true);
    }
    m_available = true;

    if (nfc_get_fd(NFC_CHANNEL_TYPE_PUBLIC, &nfcFD) != NFC_RESULT_SUCCESS) {
        qWarning() << Q_FUNC_INFO << "Could not get NFC FD";
        return;
    }

    nfcNotifier = new QSocketNotifier(nfcFD, QSocketNotifier::Read);
    qQNXNFCDebug() << "Connecting SocketNotifier" << connect(nfcNotifier, SIGNAL(activated(int)), this, SLOT(newNfcEvent(int)));
}

QNXNFCManager::~QNXNFCManager()
{
    nfc_disconnect();

    if (nfcNotifier)
        delete nfcNotifier;
}

QList<QNdefMessage> QNXNFCManager::decodeTargetMessage(nfc_target_t *target)
{
    unsigned int messageCount;
    QList<QNdefMessage> ndefMessages;

    if (nfc_get_ndef_message_count(target, &messageCount) != NFC_RESULT_SUCCESS)
        qWarning() << Q_FUNC_INFO << "Could not get ndef message count";

    for (unsigned int i=0; i<messageCount; i++) {
        nfc_ndef_message_t *nextMessage;
        if (nfc_get_ndef_message(target, i, &nextMessage) != NFC_RESULT_SUCCESS) {
            qWarning() << Q_FUNC_INFO << "Could not get ndef message";
        } else {
            QNdefMessage newNdefMessage;
            unsigned int recordCount;
            nfc_get_ndef_record_count(nextMessage, &recordCount);
            for (unsigned int j=0; j<recordCount; j++) {
                nfc_ndef_record_t *newRecord;
                char *recordType;
                uchar_t *payLoad;
                char *recordId;
                size_t payLoadSize;
                tnf_type_t typeNameFormat;

                nfc_get_ndef_record(nextMessage, j, &newRecord);

                nfc_get_ndef_record_type(newRecord, &recordType);
                QNdefRecord newNdefRecord;
                newNdefRecord.setType(QByteArray(recordType));

                nfc_get_ndef_record_payload(newRecord, &payLoad, &payLoadSize);
                newNdefRecord.setPayload(QByteArray(reinterpret_cast<const char*>(payLoad), payLoadSize));

                nfc_get_ndef_record_id(newRecord, &recordId);
                newNdefRecord.setId(QByteArray(recordId));

                nfc_get_ndef_record_tnf(newRecord, &typeNameFormat);
                QNdefRecord::TypeNameFormat recordTnf;
                switch (typeNameFormat) {
                case NDEF_TNF_WELL_KNOWN: recordTnf = QNdefRecord::NfcRtd; break;
                case NDEF_TNF_EMPTY: recordTnf = QNdefRecord::Empty; break;
                case NDEF_TNF_MEDIA: recordTnf = QNdefRecord::Mime; break;
                case NDEF_TNF_ABSOLUTE_URI: recordTnf = QNdefRecord::Uri; break;
                case NDEF_TNF_EXTERNAL: recordTnf = QNdefRecord::ExternalRtd; break;
                case NDEF_TNF_UNKNOWN: recordTnf = QNdefRecord::Unknown; break;
                    //TODO add the rest
                case NDEF_TNF_UNCHANGED: recordTnf = QNdefRecord::Unknown; break;
                }

                newNdefRecord.setTypeNameFormat(recordTnf);
                qQNXNFCDebug() << "Adding NFC record";
                newNdefMessage << newNdefRecord;
                delete recordType;
                delete payLoad;
                delete recordId;
            }
            ndefMessages.append(newNdefMessage);
        }
    }
    return ndefMessages;
}

void QNXNFCManager::newNfcEvent(int fd)
{
    nfc_event_t *nfcEvent;
    nfc_event_type_t nfcEventType;

    if (nfc_read_event(fd, &nfcEvent) != NFC_RESULT_SUCCESS) {
        qWarning() << Q_FUNC_INFO << "Could not read NFC event";
        return;
    }

    if (nfc_get_event_type(nfcEvent, &nfcEventType) != NFC_RESULT_SUCCESS) {
        qWarning() << Q_FUNC_INFO << "Could not get NFC event type";
        return;
    }

    switch (nfcEventType) { //TODO handle all the events
    case NFC_TAG_READWRITE_EVENT: qQNXNFCDebug() << "NFC read write event"; nfcReadWriteEvent(nfcEvent); break;
    case NFC_OFF_EVENT: qQNXNFCDebug() << "NFC is off"; break;
    case NFC_ON_EVENT: qQNXNFCDebug() << "NFC is on"; break;
    case NFC_HANDOVER_COMPLETE_EVENT: qQNXNFCDebug() << "NFC handover event"; break;
    case NFC_HANDOVER_DETECTED_EVENT: qQNXNFCDebug() << "NFC Handover detected"; break;
    case NFC_SNEP_CONNECTION_EVENT: qQNXNFCDebug() << "NFC SNEP detected"; break;
    case NFC_LLCP_READ_COMPLETE_EVENT: qQNXNFCDebug() << "Read complete event"; break;
    case NFC_LLCP_WRITE_COMPLETE_EVENT: qQNXNFCDebug() << "Write complete event"; break;
    case NFC_LLCP_CONNECTION_EVENT: qQNXNFCDebug() << "LLCP connection event"; break;
    default: qQNXNFCDebug() << "Got NFC event" << nfcEventType; break;
    }

    nfc_free_event (nfcEvent);
}

void QNXNFCManager::nfcReadWriteEvent(nfc_event_t *nfcEvent)
{
    nfc_target_t *target;

    if (nfc_get_target(nfcEvent, &target) != NFC_RESULT_SUCCESS) {
        qWarning() << Q_FUNC_INFO << "Could not retrieve NFC target";
        return;
    }
    tag_variant_type_t variant;
    nfc_get_tag_variant(target, &variant);
    qQNXNFCDebug() << "Variant:" << variant;

    QList<QNdefMessage> targetMessages = decodeTargetMessage(target);
    NearFieldTarget<QNearFieldTarget> *bbNFTarget = new NearFieldTarget<QNearFieldTarget>(this, target, targetMessages);
    emit targetDetected(bbNFTarget, targetMessages);
//    for (int i=0; i< targetMessages.count(); i++) {
//        for (int j=0; j<ndefMessageHandlers.count(); j++) {
//            //ndefMessageHandlers.at(j).second.invoke(ndefMessageHandlers.at(j).first,
//            //      Q_ARG(const QNdefMessage&, targetMessages.at(i)), Q_ARG(NearFieldTarget*, bbNFTarget));
//        }
//    }
}

//void QNXNFCManager::startBTHandover()
//{

//}

bool QNXNFCManager::startTargetDetection()
{
    if (nfc_register_tag_readerwriter(TAG_TYPE_ALL) == NFC_RESULT_SUCCESS) {
        return true;
    } else {
        qWarning() << Q_FUNC_INFO << "Could not start Target detection";
        return false;
    }
}

//void QNXNFCManager::handleInvoke(const bb::system::InvokeRequest& request)
//{
//    /*QString action = request.action(); // The action to be performed
//    QString mime = request.mimeType(); // Content type of the data
//    QString uri = request.uri().toString(); // Location of data – out of band
//    QString data = QString(request.data()); // In-band data
//    qQNXNFCDebug() << action << mime << uri << data;*/
//}

QTNFC_END_NAMESPACE
