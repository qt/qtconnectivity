/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
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

#include "qbluetoothservicediscoveryagent_p.h"

#include <QUrl>
#include <QtEndian>

#include <arpa/inet.h>
#include <netinet/in.h>

QTBLUETOOTH_BEGIN_NAMESPACE

QBluetoothServiceDiscoveryAgentPrivate::QBluetoothServiceDiscoveryAgentPrivate(const QBluetoothAddress &address)
    : error(QBluetoothServiceDiscoveryAgent::NoError)
    , state(Inactive)
    , deviceAddress(address)
    , deviceDiscoveryAgent(0)
    , mode(QBluetoothServiceDiscoveryAgent::MinimalDiscovery)
    , singleDevice(false)
    , m_sdpAgent(NULL)
    , m_filter(NULL)
    , m_attributes(NULL)
{

}

QBluetoothServiceDiscoveryAgentPrivate::~QBluetoothServiceDiscoveryAgentPrivate()
{
  qDebug() << "QBluetoothServiceDiscoveryAgent being destroyed";
    delete m_filter;
    delete m_attributes;
    delete m_sdpAgent;
}

void QBluetoothServiceDiscoveryAgentPrivate::start(const QBluetoothAddress &address)
{
    Q_Q(QBluetoothServiceDiscoveryAgent);
    TRAPD(err, startL(address));
    if (err != KErrNone && singleDevice) {
        qDebug() << "Service discovery start failed" << err;
        error = QBluetoothServiceDiscoveryAgent::UnknownError;
        emit q->error(error);
    }
    qDebug() << "Service discovery started";
}

void QBluetoothServiceDiscoveryAgentPrivate::startL(const QBluetoothAddress &address)
{
    initL(address);    

    if (uuidFilter.isEmpty()) {
        m_filter->AddL(QBluetoothUuid::PublicBrowseGroup);
    } else {
        foreach (const QBluetoothUuid &uuid, uuidFilter) {
            if (uuid.minimumSize() == 2) {
                TUUID sUuid(uuid.toUInt16());
                m_filter->AddL(sUuid);
            } else if (uuid.minimumSize() == 4) {
                TUUID sUuid(uuid.toUInt32());
                m_filter->AddL(sUuid);
            } else if (uuid.minimumSize() == 16) {
                TUint32 *dataPointer = (TUint32*)uuid.toUInt128().data;
                TUint32 hH = qToBigEndian<quint32>(*(dataPointer++));
                TUint32 hL = qToBigEndian<quint32>(*(dataPointer++));
                TUint32 lH = qToBigEndian<quint32>(*(dataPointer++));
                TUint32 lL = qToBigEndian<quint32>(*(dataPointer));
                TUUID sUuid(hH, hL, lH, lL);
                m_filter->AddL(sUuid);
            } else {
                // filter size can be 0 on error cases, searching all services
                m_filter->AddL(QBluetoothUuid::PublicBrowseGroup);
            }
        }
    }
    m_sdpAgent->SetRecordFilterL(*m_filter);
    m_sdpAgent->NextRecordRequestL();
    qDebug() << "Service discovery started, in startL";
}

void QBluetoothServiceDiscoveryAgentPrivate::stop()
{
  qDebug() << "Stop discovery called";
    delete m_sdpAgent;
    m_sdpAgent = NULL;
    delete m_filter;
    m_filter = NULL;
    delete m_attributes;
    m_attributes = NULL;
    Q_Q(QBluetoothServiceDiscoveryAgent);
    emit q->canceled();
}

void QBluetoothServiceDiscoveryAgentPrivate::initL(const QBluetoothAddress &address)
{
    TBTDevAddr btAddress(address.toUInt64());
    stop();

    //Trapped in Start
    m_sdpAgent = q_check_ptr(CSdpAgent::NewL(*this, btAddress));
    m_filter = q_check_ptr(CSdpSearchPattern::NewL());
    m_attributes = q_check_ptr(CSdpAttrIdMatchList::NewL());
    m_attributes->AddL(KAttrRangeAll);

}

void QBluetoothServiceDiscoveryAgentPrivate::NextRecordRequestComplete(TInt aError, TSdpServRecordHandle aHandle, TInt aTotalRecordsCount)
{
  qDebug() << "NextRecordRequestComplete";
    Q_Q(QBluetoothServiceDiscoveryAgent);
    if (aError == KErrNone && aTotalRecordsCount > 0 && m_sdpAgent && m_attributes) {
        // request attributes
        TRAPD(err, m_sdpAgent->AttributeRequestL(aHandle, *m_attributes));
        if (err && singleDevice) {
            error = QBluetoothServiceDiscoveryAgent::UnknownError;
            emit q->error(error);
        }
    } else if (aError == KErrEof) {
        _q_serviceDiscoveryFinished();
    } else {
        _q_serviceDiscoveryFinished();
    }
}

void QBluetoothServiceDiscoveryAgentPrivate::AttributeRequestResult(TSdpServRecordHandle, TSdpAttributeID aAttrID, CSdpAttrValue *aAttrValue)
{
  qDebug() << "AttributeRequestResult";
    Q_Q(QBluetoothServiceDiscoveryAgent);
    m_currentAttributeId = aAttrID;
    TRAPD(err, aAttrValue->AcceptVisitorL(*this));
    if (m_stack.size() != 1) {
        if(singleDevice)
            {
            error = QBluetoothServiceDiscoveryAgent::UnknownError;
            emit q->error(error);
            }
        delete aAttrValue;
        return;
    }

    m_serviceInfo.setAttribute(aAttrID, m_stack.pop());

    if (err != KErrNone && singleDevice) {
        error = QBluetoothServiceDiscoveryAgent::UnknownError;
        emit q->error(error);
    }
    delete aAttrValue;
}

void QBluetoothServiceDiscoveryAgentPrivate::AttributeRequestComplete(TSdpServRecordHandle, TInt aError)
{
  qDebug() << "AttributeRequestComplete";
    Q_Q(QBluetoothServiceDiscoveryAgent);

    if (aError == KErrNone && m_sdpAgent) {
        m_serviceInfo.setDevice(discoveredDevices.at(0));
        discoveredServices.append(m_serviceInfo);
        m_serviceInfo = QBluetoothServiceInfo();
        if (discoveredServices.last().isValid())
            {
            emit q->serviceDiscovered(discoveredServices.last());
            }
        TRAPD(err, m_sdpAgent->NextRecordRequestL());
        if (err != KErrNone && singleDevice) {
            error = QBluetoothServiceDiscoveryAgent::UnknownError;
            emit q->error(error);
        }
    } else if (aError != KErrEof && singleDevice) {
        error = QBluetoothServiceDiscoveryAgent::UnknownError;
        emit q->error(error);
    }

    
}

void QBluetoothServiceDiscoveryAgentPrivate::VisitAttributeValueL(CSdpAttrValue &aValue, TSdpElementType aType)
{
  qDebug() << "VisitAttributeValueL";
    QVariant var;
    TUint datasize = aValue.DataSize();
    
    switch (aType) {
    case ETypeNil:
        break;
    case ETypeUint:
        if (datasize == 8) {
            TUint64 value;
            aValue.Uint64(value);
            var = QVariant::fromValue(value);
        } else
            var = QVariant::fromValue(aValue.Uint());
        break;
    case ETypeInt:
            var = QVariant::fromValue(aValue.Int());
        break;
    case ETypeUUID: {
        TPtrC8 shortForm(aValue.UUID().ShortestForm());
        if (shortForm.Size() == 2) {
            QBluetoothUuid uuid(ntohs(*reinterpret_cast<const quint16 *>(shortForm.Ptr())));
            var = QVariant::fromValue(uuid);
        } else if (shortForm.Size() == 4) {
            QBluetoothUuid uuid(ntohl(*reinterpret_cast<const quint32 *>(shortForm.Ptr())));
            var = QVariant::fromValue(uuid);
        } else if (shortForm.Size() == 16) {
            QBluetoothUuid uuid(*reinterpret_cast<const quint128 *>(shortForm.Ptr()));
            var = QVariant::fromValue(uuid);
        }
        break;
    }
    case ETypeString: {
        TPtrC8 stringBuffer = aValue.Des();
        var = QVariant::fromValue(QString::fromLocal8Bit(reinterpret_cast<const char *>(stringBuffer.Ptr()), stringBuffer.Size()));
        break;
    }
    case ETypeBoolean:
        var = QVariant::fromValue(static_cast<bool>(aValue.Bool()));
        break;
    case ETypeDES:
        m_stack.push(QVariant::fromValue(QBluetoothServiceInfo::Sequence()));
        break;
    case ETypeDEA:
        m_stack.push(QVariant::fromValue(QBluetoothServiceInfo::Alternative()));
        break;
    case ETypeURL: {
        TPtrC8 stringBuffer = aValue.Des();
        var = QVariant::fromValue(QUrl(QString::fromLocal8Bit(reinterpret_cast<const char *>(stringBuffer.Ptr()), stringBuffer.Size())));
        break;
    }
    case ETypeEncoded:
        qWarning() << "Don't know how to handle encoded type.";
        break;
    default:
        qWarning() << "Don't know how to handle type" << aType;
    }

    if (aType != ETypeDES && aType != ETypeDEA) {
        if (m_stack.size() == 0) {
            // single value attribute, just push onto stack
            m_stack.push(var);
        } else if (m_stack.size() >= 1) {
            // sequence or alternate attribute, add non-DES -DEA values to DES or DEA
            if (m_stack.top().canConvert<QBluetoothServiceInfo::Sequence>()) {
                QBluetoothServiceInfo::Sequence *sequence = static_cast<QBluetoothServiceInfo::Sequence *>(m_stack.top().data());
                sequence->append(var);
            } else if (m_stack.top().canConvert<QBluetoothServiceInfo::Alternative>()) {
                QBluetoothServiceInfo::Alternative *alternative = static_cast<QBluetoothServiceInfo::Alternative *>(m_stack.top().data());
                alternative->append(var);
            } else {
                qWarning("Unknown type in the QVariant, should be either a QBluetoothServiceInfo::Sequence or an QBluetoothServiceInfo::Alternative");
            }
        }
    }
}

void QBluetoothServiceDiscoveryAgentPrivate::StartListL(CSdpAttrValueList &)
{

}

void QBluetoothServiceDiscoveryAgentPrivate::EndListL()
{
    if (m_stack.size() > 1) {
        // finished a sequence or alternative add it to the parent sequence or alternative
        QVariant var = m_stack.pop();
        if (m_stack.top().canConvert<QBluetoothServiceInfo::Sequence>()) {
            QBluetoothServiceInfo::Sequence *sequence = static_cast<QBluetoothServiceInfo::Sequence *>(m_stack.top().data());
            sequence->append(var);
        } else if (m_stack.top().canConvert<QBluetoothServiceInfo::Alternative>()) {
            QBluetoothServiceInfo::Alternative *alternative = static_cast<QBluetoothServiceInfo::Alternative *>(m_stack.top().data());
            alternative->append(var);
        } else {
            qWarning("Unknown type in the QVariant, should be either a QBluetoothServiceInfo::Sequence or an QBluetoothServiceInfo::Alternative");
        }
    }
}

QTBLUETOOTH_END_NAMESPACE
