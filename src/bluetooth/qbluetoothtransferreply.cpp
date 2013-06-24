/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
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


#include "qbluetoothtransferreply.h"
#include "qbluetoothtransferreply_p.h"
#include "qbluetoothaddress.h"

QT_BEGIN_NAMESPACE_BLUETOOTH

/*!
    \class QBluetoothTransferReply
    \inmodule QtBluetooth
    \brief The QBluetoothTransferReply class stores the response for a data
    transfer request.

    In additional to a copy of the QBluetoothTransferRequest object used to create the request,
    QBluetoothTransferReply contains the contents of the reply itself.

    QBluetoothTransferReply is a sequential-access QIODevice, which means that once data is read
    from the object, it is no longer kept by the device. It is the application's
    responsibility to keep this data if it needs to. Whenever more data is received and processed,
    the readyRead() signal is emitted.

    The downloadProgress() signal is also emitted when data is received, but the number of bytes
    contained in it may not represent the actual bytes received, if any transformation is done to
    the contents (for example, decompressing and removing the protocol overhead).

    Even though QBluetoothTransferReply is a QIODevice connected to the contents of the reply, it
    emits the uploadProgress() signal, which indicates the progress of the upload for
    operations that have such content.
*/

/*!
    \enum QBluetoothTransferReply::TransferError

    This enum describes the type of error that occurred

    \value NoError          No error.
    \value UnknownError     Unknown error, no better enum available
    \value FileNotFoundError Unable to open the file specified
    \value HostNotFoundError Unable to connect to the target host
    \value UserCanceledTransferError User terminated the transfer
*/



/*!
    \fn QBluetoothTransferReply::abort()

    Aborts this reply.
*/
void QBluetoothTransferReply::abort()
{

}

/*!
    \fn void QBluetoothTransferReply::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)

    This signal is emitted whenever data is received. The \a bytesReceived parameter contains the
    total number of bytes received so far out of \a bytesTotal expected for the entire transfer.
*/

/*!
    \fn void QBluetoothTransferReply::finished(QBluetoothTransferReply *reply)

    This signal is emitted when the transfer is complete for \a reply.
*/

/*!
    \fn void QBluetoothTransferReply::uploadProgress(qint64 bytesSent, qint64 bytesTotal)

    This signal is emitted whenever data is sent. The \a bytesSent parameter contains the total
    number of bytes sent so far out of \a bytesTotal.
*/

/*!
    Constructs a new QBluetoothTransferReply with \a parent.
*/
QBluetoothTransferReply::QBluetoothTransferReply(QObject *parent)
:   QObject(parent), d_ptr(new QBluetoothTransferReplyPrivate)
{
    qRegisterMetaType<QBluetoothTransferReply*>("QBluetoothTransferReply");
}

/*!
    Destroys the QBluetoothTransferReply object.
*/
QBluetoothTransferReply::~QBluetoothTransferReply()
{
    delete d_ptr;
}

/*!
    Returns the attribute associated with \a code. If the attribute has not been set, it
    returns an invalid QVariant.
*/
QVariant QBluetoothTransferReply::attribute(QBluetoothTransferRequest::Attribute code) const
{
    Q_D(const QBluetoothTransferReply);
    return d->m_attributes[code];
}

/*!
   \fn bool QBluetoothTransferReply::isFinished() const

    Returns true if this reply has finished, otherwise false.
*/

/*!
   \fn bool QBluetoothTransferReply::isRunning() const

    Returns true if this reply is running, otherwise false.
*/

/*!
    Returns the QBluetoothTransferManager that was used to create this QBluetoothTransferReply
    object.
*/
QBluetoothTransferManager *QBluetoothTransferReply::manager() const
{
    Q_D(const QBluetoothTransferReply);
    return d->m_manager;
}

/*!
    Returns the type of operation that this reply is for.
*/
QBluetoothTransferManager::Operation QBluetoothTransferReply::operation() const
{
    Q_D(const QBluetoothTransferReply);
    return d->m_operation;
}

/*!
    Sets the operation of this QBluetoothTransferReply to \a operation.
*/
void QBluetoothTransferReply::setOperation(QBluetoothTransferManager::Operation operation)
{
    Q_D(QBluetoothTransferReply);
    d->m_operation = operation;
}

/*!
  \fn QBluetoothTransferReply::setAttribute(QBluetoothTransferRequest::Attribute code, const QVariant &value)

    Set the attribute associated with the \a code to \a value.
*/
void QBluetoothTransferReply::setAttribute(QBluetoothTransferRequest::Attribute code, const QVariant &value)
{
    Q_D(QBluetoothTransferReply);
    d->m_attributes.insert(code, value);
}

/*!
  \fn QBluetoothTransferReply::setManager(QBluetoothTransferManager *manager)

  Set the reply's manager to the \a manager.
*/

void QBluetoothTransferReply::setManager(QBluetoothTransferManager *manager)
{
    Q_D(QBluetoothTransferReply);
    d->m_manager = manager;
}

/*!
  \fn TransferError QBluetoothTransferReply::error() const

  The error code of the error that occurred.
*/

/*!
  \fn QString QBluetoothTransferReply::errorString() const

  String describing the error. Can be displayed to the user.
*/

QBluetoothTransferReplyPrivate::QBluetoothTransferReplyPrivate()
{
}

#include "moc_qbluetoothtransferreply.cpp"

QT_END_NAMESPACE_BLUETOOTH
