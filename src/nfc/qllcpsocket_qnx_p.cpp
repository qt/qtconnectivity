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
#include <unistd.h>

QT_BEGIN_NAMESPACE_NFC

QLlcpSocketPrivate::QLlcpSocketPrivate(QLlcpSocket *q)
    :   q_ptr(q), m_conListener(0), m_state(QLlcpSocket::UnconnectedState), m_server(false)
{
}

QLlcpSocketPrivate::~QLlcpSocketPrivate()
{
    disconnectFromService();
}

void QLlcpSocketPrivate::connectToService(QNearFieldTarget *target, const QString &serviceUri)
{
    Q_UNUSED(target)
    if (m_state != QLlcpSocket::UnconnectedState) {
        qWarning() << Q_FUNC_INFO << "socket is already connected";
        return;
    }

    m_state = QLlcpSocket::ConnectingState;
    if (nfc_llcp_register_connection_listener(CLIENT, 0, serviceUri.toLocal8Bit().constData(),
                                              &m_conListener) != NFC_RESULT_SUCCESS) {
        qWarning() << Q_FUNC_INFO << "could not register for connection listener";
        return;
    }

    QNXNFCManager::instance()->registerLLCPConnection(m_conListener, this);

    qQNXNFCDebug() << "Connecting client socket" << serviceUri << m_conListener;
    connect(QNXNFCManager::instance(), SIGNAL(llcpDisconnected()), this, SLOT(disconnectFromService()));
}

void QLlcpSocketPrivate::disconnectFromService()
{
    Q_Q(QLlcpSocket);
    QNXNFCManager::instance()->unregisterTargetLost(this);
    qQNXNFCDebug() << "Shutting down LLCP socket";
    if (!m_server && nfc_llcp_unregister_connection_listener(m_conListener) != NFC_RESULT_SUCCESS) {
        qWarning() << Q_FUNC_INFO << "Error when trying to close LLCP socket";
    }
    QNXNFCManager::instance()->unregisterLLCPConnection(m_conListener);
    disconnect(QNXNFCManager::instance(), SIGNAL(llcpDisconnected()), this, SLOT(disconnectFromService()));

    q->disconnected();
    m_conListener = 0;
    m_state = QLlcpSocket::UnconnectedState;
}

bool QLlcpSocketPrivate::bind(quint8 port)
{
    Q_UNUSED(port);

    m_state = QLlcpSocket::ConnectedState;
    m_server = true;
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
    return writeDatagram(datagram.constData(), datagram.size());
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
    if (socketState != Idle) {
        m_writeQueue.append(QByteArray(data, len));
        return len;
    } else {
        socketState = Writing;
        qQNXNFCDebug() << "LLCP write";
        nfc_result_t res = nfc_llcp_write(m_target, (uchar_t*)data, (size_t)len);
        if (res == NFC_RESULT_SUCCESS) {
            return len;
        } else {
            qWarning() << Q_FUNC_INFO << "Error writing to LLCP socket. Error" << res;
            enteringIdle();
            return -1;
        }
    }
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

    m_state = QLlcpSocket::ConnectedState;
    emit q->connected();
    qQNXNFCDebug() << "Socket connected";

    unsigned int targetId;
    nfc_get_target_connection_id(target, &targetId);
    QNXNFCManager::instance()->requestTargetLost(this, targetId);
    enteringIdle();
}

void QLlcpSocketPrivate::targetLost()
{
    disconnectFromService();
    qQNXNFCDebug() << "LLCP target lost...socket disconnected";
}

void QLlcpSocketPrivate::dataRead(QByteArray& data)
{
    Q_Q(QLlcpSocket);
    if (!data.isEmpty()) {
        m_receivedDatagrams.append(data);
        emit q->readyRead();
    }
    socketState = Idle;
    enteringIdle();
}

void QLlcpSocketPrivate::dataWritten()
{
    enteringIdle();
}

void QLlcpSocketPrivate::read()
{
    if (socketState != Idle) {
        qQNXNFCDebug() << "Trying to read but socket state not in idle..abort";
        return;
    }
    socketState = Reading;
    qQNXNFCDebug() << "LLCP read";
    if (nfc_llcp_read(m_target, 128) != NFC_RESULT_SUCCESS) {
        qWarning() << Q_FUNC_INFO << "Could not register for reading";
        socketState = Idle;
    }
}

void QLlcpSocketPrivate::enteringIdle()
{
    qQNXNFCDebug() << "entering idle; Socket state:" << socketState;
    socketState = Idle;
    if (m_state == QLlcpSocket::ConnectedState) {
        if (m_writeQueue.isEmpty()) {
            qQNXNFCDebug() << "Write queue empty, reading in 50ms";
            QTimer::singleShot(50, this, SLOT(read()));
        } else {
            qQNXNFCDebug() << "Write first package in queue";
            writeDatagram(m_writeQueue.takeFirst());
        }
    }
}

QT_END_NAMESPACE_NFC

