/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Mobility Components.
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


#include "qbluetoothtransferreply_symbian_p.h"
#include "symbian/utils_symbian_p.h"
#include "qbluetoothservicediscoveryagent.h"
#include "qbluetoothserviceinfo.h"
#include "qbluetoothuuid.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QDir>

QBluetoothTransferReplySymbian::QBluetoothTransferReplySymbian(QIODevice *input,
    QBluetoothTransferManager::Operation operation, QObject *parent)
    : QBluetoothTransferReply(parent)
    , CActive(EPriorityStandard)
    , m_source(input)
    , m_running(false)
    , m_finished(false)
    , m_client(NULL)
    , m_object(NULL)
    , m_error(QBluetoothTransferReply::NoError)
    , m_errorStr()
    , m_timer(new QTimer(this))
{
    setOperation(operation);

    //add this active object to scheduler
    CActiveScheduler::Add(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(updateProgress()));

}

/*!
    Destroys the QBluetoothTransferReply object.
*/
QBluetoothTransferReplySymbian::~QBluetoothTransferReplySymbian()
{
    Cancel();

    if (m_timer->isActive())
        m_timer->stop();

    delete m_object;
    m_object = NULL;

    delete m_client;
    m_client = NULL;
}

void QBluetoothTransferReplySymbian::setAddress(const QBluetoothAddress &address)
{
    m_address = address;
}

void QBluetoothTransferReplySymbian::serviceDiscovered(const QBluetoothServiceInfo &info)
{
    m_port = info.serverChannel();
}

void QBluetoothTransferReplySymbian::updateProgress()
{
    if (m_object) {
        qint64 sent = m_object->BytesSent();
        emit uploadProgress(sent, m_fileSize);
    }
}

void QBluetoothTransferReplySymbian::serviceDiscoveryFinished()
{
    QMetaObject::invokeMethod(this, "startTransfer", Qt::QueuedConnection);
}

bool QBluetoothTransferReplySymbian::start()
{
    if (m_address.isNull())
        return false;

    m_running = true;

    m_discoveryAgent = new QBluetoothServiceDiscoveryAgent(m_address);

    connect(m_discoveryAgent, SIGNAL(serviceDiscovered(QBluetoothServiceInfo)),
            this, SLOT(serviceDiscovered(QBluetoothServiceInfo)));
    connect(m_discoveryAgent, SIGNAL(finished()), this, SLOT(serviceDiscoveryFinished()));
    // Automatically delete agent when device discovery finishes.
    connect(m_discoveryAgent, SIGNAL(finished()), this, SLOT(deleteLater()));

    m_discoveryAgent->setUuidFilter(QBluetoothUuid(QBluetoothUuid::ObexObjectPush));
    m_discoveryAgent->start();

    return true;
}

void QBluetoothTransferReplySymbian::startTransfer()
{
    TObexBluetoothProtocolInfo protocolInfo;
    TBTDevAddr deviceAddress(m_address.toUInt64());

    protocolInfo.iTransport.Copy(KBTSProtocol);
    protocolInfo.iAddr.SetBTAddr(deviceAddress);
    protocolInfo.iAddr.SetPort(m_port); // TODO: set port if needed

    if ( m_client ) {
        delete m_client;
        m_client = NULL;
    }

    TRAPD( err, m_client = CObexClient::NewL(protocolInfo));
    if (err) {
        qDebug() << "Error in" << __FUNCTION__ << err;
        m_error = err == KErrNotFound ? HostNotFoundError: UnknownError;
        return;
    }

    m_state = EConnecting;
    m_client->Connect( iStatus );

    SetActive();
}

/*!
    Returns true if this reply has finished; otherwise returns false.
*/
bool QBluetoothTransferReplySymbian::isFinished() const
{
    return m_finished;
}

/*!
    Returns true if this reply is running; otherwise returns false.
*/
bool QBluetoothTransferReplySymbian::isRunning() const
{
    return m_running;
}

void QBluetoothTransferReplySymbian::abort()
{
    if (m_timer->isActive())
        m_timer->stop();
    m_state = EIdle;
    m_running = false;
    // Deleting obexclient is the only way to cancel active requests
    delete m_client;
    m_client = NULL;

    delete m_object;
    m_object = NULL;
}

QBluetoothTransferReply::TransferError QBluetoothTransferReplySymbian::error() const
{
    return m_error;
}

QString QBluetoothTransferReplySymbian::errorString() const
{
    return m_errorStr;
}

void QBluetoothTransferReplySymbian::DoCancel()
{
    m_running = false;
    // Deleting obexclient is the only way to cancel active requests
    if ( m_client ) {
        delete m_client;
        m_client = NULL;
        m_state = EIdle;
    }
    if (m_timer->isActive())
        m_timer->stop();
    m_error = UserCanceledTransferError;
}

void QBluetoothTransferReplySymbian::RunL()
{
    if (iStatus.Int() != KErrNone) {
        abort();
        emit finished(this);
    }

    switch ( m_state ) {
        case EConnecting: {
            m_state = ESending;
            QFile *file = qobject_cast<QFile *>(m_source);

            QString filename;
            if (file) {
                QFileInfo info(*file);
                m_fileSize = info.size();
                filename = QDir::toNativeSeparators(info.absoluteFilePath());
            } else {
                if (copyToTempFile(m_tempfile, m_source)) {
                   QFileInfo info(*m_tempfile);
                   m_fileSize = info.size();
                   filename = QDir::toNativeSeparators(info.absoluteFilePath());
                } else {
                    m_state = EDisconnecting;
                    disconnect();
                }
            }
            sendObject(filename);
            break;
        }
        case ESending: {
            m_state = EDisconnecting;
            disconnect();
            break;
        }
        case EDisconnecting: {
            m_state = EIdle;
            m_finished = true;
            m_running = false;
            emit finished(this);
            break;
        }
        case EIdle:
        default:
            break;
    }

}

void QBluetoothTransferReplySymbian::sendObject(QString filename)
{
    delete m_object;
    m_object = NULL;
    TRAPD(err, m_object = CObexFileObject::NewL());
    if (!err) {
        TPtrC16 str(reinterpret_cast<const TUint16*>(filename.utf16()));
        TRAPD(error, m_object->InitFromFileL( str ));
        if (!error) {
            m_client->Put( *m_object, iStatus );
            m_timer->start(1000);
            emit uploadProgress(0, m_fileSize);
            SetActive();
        } else {
            qDebug() << "Error in" << __FUNCTION__ << error;
            m_error = error == KErrNotFound ? FileNotFoundError: UnknownError;
            disconnect();
            emit finished(this);
        }
    }
}

void QBluetoothTransferReplySymbian::disconnect()
{
    if ( m_state == EDisconnecting || m_error != NoError) {
        if (m_timer->isActive())
            m_timer->stop();
        delete m_object;
        m_object = NULL;
        m_client->Disconnect( iStatus );
        SetActive();
    }
}

bool QBluetoothTransferReplySymbian::copyToTempFile(QIODevice *to, QIODevice *from)
{
    char *block = new char[4096];

    while (!from->atEnd()) {
        int size = from->read(block, 4096);
        if (size != to->write(block, size)) {
            return false;
        }
    }

    delete[] block;
    return true;
}

#include "moc_qbluetoothtransferreply_symbian_p.cpp"
