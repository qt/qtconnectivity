/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
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

#include "qbluetoothtransfermanager.h"
#include "qbluetoothtransferrequest.h"

QTBLUETOOTH_BEGIN_NAMESPACE

/*!
    \class QBluetoothTransferManager
    \inmodule QtBluetooth
    \brief The QBluetoothTransferManager class transfers data to another device
    using Object Push Profile (OPP).

    QBluetoothTransferManager uses OBEX to send get and put commands to remote devices.
*/

/*!
    \enum QBluetoothTransferManager::Operation

    This enum describes the type of operation that a transfer request is for.

    \value GetOperation     The get operation is used to retrieve an object from a remote device. Not implemented.
    \value PutOperation     The put operation is used to send an object to a remote device.
*/

/*!
    \fn QBluetoothTransferReply *QBluetoothTransferManager::put(const QBluetoothTransferRequest &request, QIODevice *data)

    Sends the contents of \a data to the remote device identified by \a request, and returns a new
    QBluetoothTransferReply that can be used to track the request's progress.
*/



/*!
    \fn void QBluetoothTransferManager::finished(QBluetoothTransferReply *reply)

    This signal is emitted when the transfer for \a reply finishes.
*/

/*!
    Constructs a new QBluetoothTransferManager with \a parent.
*/
QBluetoothTransferManager::QBluetoothTransferManager(QObject *parent)
:   QObject(parent)
{
}

/*!
    Destroys the QBluetoothTransferManager.
*/
QBluetoothTransferManager::~QBluetoothTransferManager()
{
}

#include "moc_qbluetoothtransfermanager.cpp"

QTBLUETOOTH_END_NAMESPACE
