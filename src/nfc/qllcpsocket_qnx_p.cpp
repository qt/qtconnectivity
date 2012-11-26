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

#include "qllcpsocket_qnx_p.h"

QTNFC_BEGIN_NAMESPACE

QLlcpSocketPrivate::QLlcpSocketPrivate(QLlcpSocket *q)
    :   q_ptr(q), m_state(QLlcpSocket::UnconnectedState), m_conListener(0)
{
}

QLlcpSocketPrivate::~QLlcpSocketPrivate()
{
    disconnectFromService();
}

void QLlcpSocketPrivate::connectToService(QNearFieldTarget *target, const QString &serviceUri)
{
    if (m_state != QLlcpSocket::UnconnectedState) {
        qWarning() << Q_FUNC_INFO << "socket is already connected";
        return;
    }
    m_state = QLlcpSocket::ConnectingState;
    m_conListener = nfc_llcp_register_connection_listener(CLIENT, 0, serviceUri.toLocal8Bit().constData(),
                                                       &m_conListener);

    connect(QNXNFCManager::instance(), SIGNAL(readResult(QByteArray&)), this, SLOT(dataRead(QByteArray&)));
    connect(QNXNFCManager::instance(), SIGNAL(llcpDisconnected()), this, SLOT(disconnectFromService()));
    connect(QNXNFCManager::instance(), SIGNAL(newLlcpConnection(nfc_target_t*)), this, SLOT(connected(nfc_target_t*)));
}

void QLlcpSocketPrivate::disconnectFromService()
{
    if (nfc_llcp_unregister_connection_listener(m_conListener) != NFC_RESULT_SUCCESS) {
        qWarning() << Q_FUNC_INFO << "Error when trying to close LLCP socket";
    }
    disconnect(QNXNFCManager::instance(), SIGNAL(readResult(QByteArray&)), this, SLOT(dataRead(QByteArray&)));
    disconnect(QNXNFCManager::instance(), SIGNAL(llcpDisconnected()), this, SLOT(disconnectFromService()));
    disconnect(QNXNFCManager::instance(), SIGNAL(newLlcpConnection(nfc_target_t*)), this, SLOT(connected()));

    m_conListener = 0;
    m_state = QLlcpSocket::UnconnectedState;
}

bool QLlcpSocketPrivate::bind(quint8 port)
{
    Q_UNUSED(port);

    m_state = QLlcpSocket::ConnectedState;
    connect(QNXNFCManager::instance(), SIGNAL(readResult(QByteArray&)), this, SLOT(dataRead(QByteArray&)));
    connect(QNXNFCManager::instance(), SIGNAL(llcpDisconnected()), this, SLOT(disconnectFromService()));
    connected(QNXNFCManager::instance()->getLastTarget());

    return true;
}

bool QLlcpSocketPrivate::hasPendingDatagrams() const
{
    return !m_receivedDatagrams.isEmpty();
}

qint64 QLlcpSocketPrivate::pendingDatagramSize() const
{
    if (m_receivedDatagrams.isEmpty())
        return -1;

    return m_receivedDatagrams.first().length();
}

qint64 QLlcpSocketPrivate::writeDatagram(const char *data, qint64 size)
{
    if (m_state == QLlcpSocket::ConnectedState)
        return writeData(data, size);

    return -1;
}

qint64 QLlcpSocketPrivate::writeDatagram(const QByteArray &datagram)
{
    return writeDatagram(datagram.constData(), datagram.size()-1);
}

qint64 QLlcpSocketPrivate::readDatagram(char *data, qint64 maxSize,
                                        QNearFieldTarget **target, quint8 *port)
{
    Q_UNUSED(target);
    Q_UNUSED(port);

    if (m_state == QLlcpSocket::ConnectedState)
        return readData(data, maxSize);

    return -1;
}

qint64 QLlcpSocketPrivate::writeDatagram(const char *data, qint64 size,
                                         QNearFieldTarget *target, quint8 port)
{
    Q_UNUSED(target);
    Q_UNUSED(port);

    return writeDatagram(data, size);
}

qint64 QLlcpSocketPrivate::writeDatagram(const QByteArray &datagram,
                                         QNearFieldTarget *target, quint8 port)
{
    Q_UNUSED(datagram);
    Q_UNUSED(target);
    Q_UNUSED(port);

    return writeDatagram(datagram.constData(), datagram.size()-1);
}

QLlcpSocket::SocketError QLlcpSocketPrivate::error() const
{
    return QLlcpSocket::UnknownSocketError;
}

QLlcpSocket::SocketState QLlcpSocketPrivate::state() const
{
    return m_state;
}

qint64 QLlcpSocketPrivate::readData(char *data, qint64 maxlen)
{
    if (m_receivedDatagrams.isEmpty())
        return 0;

    const QByteArray datagram = m_receivedDatagrams.takeFirst();
    qint64 size = qMin(maxlen, qint64(datagram.length()));
    memcpy(data, datagram.constData(), size);
    return size;
}

qint64 QLlcpSocketPrivate::writeData(const char *data, qint64 len)
{
    if (nfc_llcp_write(m_target, (uchar_t*)data, len) != NFC_RESULT_SUCCESS) {
        return -1;
    }

    return len;
}

qint64 QLlcpSocketPrivate::bytesAvailable() const
{
    qint64 available = 0;
    foreach (const QByteArray &datagram, m_receivedDatagrams)
        available += datagram.length();

    return available;
}

bool QLlcpSocketPrivate::canReadLine() const
{
    foreach (const QByteArray &datagram, m_receivedDatagrams) {
        if (datagram.contains('\n'))
            return true;
    }

    return false;
}

bool QLlcpSocketPrivate::waitForReadyRead(int msecs)
{
    Q_UNUSED(msecs);

    return false;
}

bool QLlcpSocketPrivate::waitForBytesWritten(int msecs)
{
    Q_UNUSED(msecs);

    return false;
}

bool QLlcpSocketPrivate::waitForConnected(int msecs)
{
    Q_UNUSED(msecs);

    return false;
}

bool QLlcpSocketPrivate::waitForDisconnected(int msecs)
{
    Q_UNUSED(msecs);

    return false;
}

void QLlcpSocketPrivate::connected(nfc_target_t *target)
{
    Q_Q(QLlcpSocket);
    m_target = target;
    if (nfc_llcp_read(m_target, 128) != NFC_RESULT_SUCCESS) {
        qWarning() << Q_FUNC_INFO << "Could not register for reading";
    }
    m_state = QLlcpSocket::ConnectedState;
    emit q->connected();
    qQNXNFCDebug() << "Socket connected";
    disconnect(QNXNFCManager::instance(), SIGNAL(newLlcpConnection(nfc_target_t*)), this, SLOT(connected(nfc_target_t*)));
}

void QLlcpSocketPrivate::dataRead(QByteArray& data)
{
    Q_Q(QLlcpSocket);
    m_receivedDatagrams.append(data);
    emit q->readyRead();
}

QTNFC_END_NAMESPACE

