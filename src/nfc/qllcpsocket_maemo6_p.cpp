/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtNfc module of the Qt Toolkit.
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

#include "qllcpsocket_maemo6_p.h"

#include "manager_interface.h"
#include "maemo6/adapter_interface_p.h"
#include "maemo6/socketrequestor_p.h"

#include <QtCore/QSocketNotifier>
#include <QtCore/QAtomicInt>

#include <errno.h>
#include <signal.h>

using namespace com::nokia::nfc;

QTNFC_BEGIN_NAMESPACE

static QAtomicInt requestorId = 0;
static const char * const requestorBasePath = "/com/nokia/nfc/llcpclient/";

QLlcpSocketPrivate::QLlcpSocketPrivate(QLlcpSocket *q)
:   q_ptr(q),
    m_connection(QDBusConnection::connectToBus(QDBusConnection::SystemBus, QUuid::createUuid())),
    m_port(0), m_socketRequestor(0), m_fd(-1), m_readNotifier(0), m_writeNotifier(0),
    m_pendingBytes(0), m_state(QLlcpSocket::UnconnectedState),
    m_error(QLlcpSocket::UnknownSocketError)
{
}

QLlcpSocketPrivate::QLlcpSocketPrivate(const QDBusConnection &connection, int fd,
                                       const QVariantMap &properties)
:   q_ptr(0), m_properties(properties), m_connection(connection), m_port(0), m_socketRequestor(0),
    m_fd(fd), m_pendingBytes(0),
    m_state(QLlcpSocket::ConnectedState), m_error(QLlcpSocket::UnknownSocketError)
{
    m_readNotifier = new QSocketNotifier(m_fd, QSocketNotifier::Read, this);
    connect(m_readNotifier, SIGNAL(activated(int)), this, SLOT(_q_readNotify()));
    m_writeNotifier = new QSocketNotifier(m_fd, QSocketNotifier::Write, this);
    connect(m_writeNotifier, SIGNAL(activated(int)), this, SLOT(_q_bytesWritten()));
}

QLlcpSocketPrivate::~QLlcpSocketPrivate()
{
    delete m_readNotifier;
    delete m_writeNotifier;
}

void QLlcpSocketPrivate::connectToService(QNearFieldTarget *target, const QString &serviceUri)
{
    Q_UNUSED(target);

    Q_Q(QLlcpSocket);

    m_state = QLlcpSocket::ConnectingState;
    emit q->stateChanged(m_state);

    initializeRequestor();

    if (m_socketRequestor) {
        m_serviceUri = serviceUri;
        m_port = 0;

        QString accessKind(QLatin1String("device.llcp.co.client:") + serviceUri);

        m_socketRequestor->requestAccess(m_requestorPath, accessKind);
    } else {
        setSocketError(QLlcpSocket::SocketResourceError);

        m_state = QLlcpSocket::UnconnectedState;
        emit q->stateChanged(m_state);
    }
}

void QLlcpSocketPrivate::disconnectFromService()
{
    Q_Q(QLlcpSocket);

    m_state = QLlcpSocket::ClosingState;
    emit q->stateChanged(m_state);

    delete m_readNotifier;
    m_readNotifier = 0;
    delete m_writeNotifier;
    m_writeNotifier = 0;
    m_pendingBytes = 0;
    ::close(m_fd);
    m_fd = -1;

    if (m_socketRequestor) {
        QString accessKind(QLatin1String("device.llcp.co.client:") + m_serviceUri);

        Manager manager(QLatin1String("com.nokia.nfc"), QLatin1String("/"), m_connection);
        QDBusObjectPath defaultAdapterPath = manager.DefaultAdapter();

        m_socketRequestor->cancelAccessRequest(m_requestorPath, accessKind);
    }

    m_state = QLlcpSocket::UnconnectedState;
    q->setOpenMode(QIODevice::NotOpen);
    emit q->stateChanged(m_state);
    emit q->disconnected();
}

bool QLlcpSocketPrivate::bind(quint8 port)
{
    initializeRequestor();

    if (!m_socketRequestor)
        return false;

    m_serviceUri.clear();
    m_port = port;

    const QString accessKind(QLatin1String("device.llcp.cl:") + QString::number(port));

    m_socketRequestor->requestAccess(m_requestorPath, accessKind);

    return waitForBound(30000);
}

bool QLlcpSocketPrivate::hasPendingDatagrams() const
{
    return !m_receivedDatagrams.isEmpty();
}

qint64 QLlcpSocketPrivate::pendingDatagramSize() const
{
    if (m_receivedDatagrams.isEmpty())
        return -1;

    if (m_state == QLlcpSocket::BoundState)
        return m_receivedDatagrams.first().length() - 1;
    else
        return m_receivedDatagrams.first().length();
}

qint64 QLlcpSocketPrivate::writeDatagram(const char *data, qint64 size)
{
    if (m_state != QLlcpSocket::ConnectedState)
        return -1;

    return writeData(data, size);
}

qint64 QLlcpSocketPrivate::writeDatagram(const QByteArray &datagram)
{
    if (m_state != QLlcpSocket::ConnectedState)
        return -1;

    if (uint(datagram.length()) > m_properties.value(QLatin1String("RemoteMIU"), 128).toUInt())
        return -1;

    return writeData(datagram.constData(), datagram.size());
}

qint64 QLlcpSocketPrivate::readDatagram(char *data, qint64 maxSize,
                                        QNearFieldTarget **target, quint8 *port)
{
    Q_UNUSED(target);

    if (m_state == QLlcpSocket::ConnectedState) {
        return readData(data, maxSize);
    } else if (m_state == QLlcpSocket::BoundState) {
        return readData(data, maxSize, port);
    }

    return -1;
}

qint64 QLlcpSocketPrivate::writeDatagram(const char *data, qint64 size,
                                         QNearFieldTarget *target, quint8 port)
{
    Q_UNUSED(target);

    if (m_state != QLlcpSocket::BoundState)
        return -1;

    if (m_properties.value(QLatin1String("RemoteMIU"), 128).toUInt() < size)
        return -1;

    if (m_properties.value(QLatin1String("LocalMIU"), 128).toUInt() < size)
        return -1;

    QByteArray datagram;
    datagram.append(port);
    datagram.append(data, size);

    m_pendingBytes += datagram.size() - 1;
    m_writeNotifier->setEnabled(true);

    return ::write(m_fd, datagram.constData(), datagram.size()) - 1;
}

qint64 QLlcpSocketPrivate::writeDatagram(const QByteArray &datagram,
                                         QNearFieldTarget *target, quint8 port)
{
    Q_UNUSED(target);

    if (m_state != QLlcpSocket::BoundState)
        return -1;

    if (m_properties.value(QLatin1String("RemoteMIU"), 128).toInt() < datagram.size())
        return -1;

    if (m_properties.value(QLatin1String("LocalMIU"), 128).toInt() < datagram.size())
        return -1;

    QByteArray d;
    d.append(port);
    d.append(datagram);

    m_pendingBytes += datagram.size() - 1;
    m_writeNotifier->setEnabled(true);

    return ::write(m_fd, d.constData(), d.size()) - 1;
}

QLlcpSocket::SocketError QLlcpSocketPrivate::error() const
{
    return m_error;
}

QLlcpSocket::SocketState QLlcpSocketPrivate::state() const
{
    return m_state;
}

qint64 QLlcpSocketPrivate::readData(char *data, qint64 maxlen, quint8 *port)
{
    if (m_receivedDatagrams.isEmpty())
        return 0;

    const QByteArray datagram = m_receivedDatagrams.takeFirst();

    if (m_state == QLlcpSocket::BoundState) {
        if (port)
            *port = datagram.at(0);

        qint64 size = qMin(maxlen, qint64(datagram.length() - 1));
        memcpy(data, datagram.constData() + 1, size);
        return size;
    } else {
        if (port)
            *port = 0;

        qint64 size = qMin(maxlen, qint64(datagram.length()));
        memcpy(data, datagram.constData(), size);
        return size;
    }
}

qint64 QLlcpSocketPrivate::writeData(const char *data, qint64 len)
{
    Q_Q(QLlcpSocket);

    qint64 remoteMiu = m_properties.value(QLatin1String("RemoteMIU"), 128).toLongLong();
    qint64 localMiu = m_properties.value(QLatin1String("LocalMIU"), 128).toLongLong();
    qint64 miu = qMin(remoteMiu, localMiu);

    m_writeNotifier->setEnabled(true);

    ssize_t wrote = ::write(m_fd, data, qMin(miu, len));
    if (wrote == -1) {
        if (errno == EAGAIN)
            return 0;

        setSocketError(QLlcpSocket::RemoteHostClosedError);
        q->disconnectFromService();
        return -1;
    }

    m_pendingBytes += wrote;
    return wrote;
}

qint64 QLlcpSocketPrivate::bytesAvailable() const
{
    qint64 available = 0;
    foreach (const QByteArray &datagram, m_receivedDatagrams)
        available += datagram.length();

    // less source port
    if (m_state == QLlcpSocket::BoundState)
        available -= m_receivedDatagrams.count();

    return available;
}

bool QLlcpSocketPrivate::canReadLine() const
{
    if (m_state == QLlcpSocket::BoundState)
        return false;

    foreach (const QByteArray &datagram, m_receivedDatagrams) {
        if (datagram.contains('\n'))
            return true;
    }

    return false;
}

bool QLlcpSocketPrivate::waitForReadyRead(int msec)
{
    if (bytesAvailable())
        return true;

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(m_fd, &fds);

    timeval timeout;
    timeout.tv_sec = msec / 1000;
    timeout.tv_usec = (msec % 1000) * 1000;

    // timeout can not be 0 or else select will return an error.
    if (0 == msec)
        timeout.tv_usec = 1000;

    int result = -1;
    // on Linux timeout will be updated by select, but _not_ on other systems.
    QElapsedTimer timer;
    timer.start();
    while (!bytesAvailable() && (-1 == msec || timer.elapsed() < msec)) {
        result = ::select(m_fd + 1, &fds, 0, 0, &timeout);
        if (result > 0)
            _q_readNotify();

        if (-1 == result && errno != EINTR) {
            setSocketError(QLlcpSocket::UnknownSocketError);
            break;
        }
    }

    return bytesAvailable();
}

bool QLlcpSocketPrivate::waitForBytesWritten(int msec)
{
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(m_fd, &fds);

    timeval timeout;
    timeout.tv_sec = msec / 1000;
    timeout.tv_usec = (msec % 1000) * 1000;

    // timeout can not be 0 or else select will return an error.
    if (0 == msec)
        timeout.tv_usec = 1000;

    int result = -1;
    // on Linux timeout will be updated by select, but _not_ on other systems.
    QElapsedTimer timer;
    timer.start();
    while (-1 == msec || timer.elapsed() < msec) {
        result = ::select(m_fd + 1, 0, &fds, 0, &timeout);
        if (result > 0)
            return true;
        if (-1 == result && errno != EINTR) {
            setSocketError(QLlcpSocket::UnknownSocketError);
            return false;
        }
    }

    // timeout expired
    return false;
}

bool QLlcpSocketPrivate::waitForConnected(int msecs)
{
    if (m_state != QLlcpSocket::ConnectingState)
        return m_state == QLlcpSocket::ConnectedState;

    QElapsedTimer timer;
    timer.start();
    while (m_state == QLlcpSocket::ConnectingState && (msecs == -1 || timer.elapsed() < msecs)) {
        if (!m_socketRequestor->waitForDBusSignal(qMax(msecs - timer.elapsed(), qint64(0)))) {
            setSocketError(QLlcpSocket::UnknownSocketError);
            break;
        }
    }

    // Possibly not needed.
    QCoreApplication::sendPostedEvents(this, QEvent::MetaCall);

    return m_state == QLlcpSocket::ConnectedState;
}

bool QLlcpSocketPrivate::waitForDisconnected(int msec)
{
    if (m_state == QLlcpSocket::UnconnectedState)
        return true;

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(m_fd, &fds);

    timeval timeout;
    timeout.tv_sec = msec / 1000;
    timeout.tv_usec = (msec % 1000) * 1000;

    // timeout can not be 0 or else select will return an error.
    if (0 == msec)
        timeout.tv_usec = 1000;

    int result = -1;
    // on Linux timeout will be updated by select, but _not_ on other systems.
    QElapsedTimer timer;
    timer.start();
    while (m_state != QLlcpSocket::UnconnectedState && (-1 == msec || timer.elapsed() < msec)) {
        result = ::select(m_fd + 1, &fds, 0, 0, &timeout);
        if (result > 0)
            _q_readNotify();

        if (-1 == result && errno != EINTR) {
            setSocketError(QLlcpSocket::UnknownSocketError);
            break;
        }
    }

    return m_state == QLlcpSocket::UnconnectedState;
}

bool QLlcpSocketPrivate::waitForBound(int msecs)
{
    if (m_state == QLlcpSocket::BoundState)
        return true;

    QElapsedTimer timer;
    timer.start();
    while (m_state != QLlcpSocket::BoundState && (msecs == -1 || timer.elapsed() < msecs)) {
        if (!m_socketRequestor->waitForDBusSignal(qMax(msecs - timer.elapsed(), qint64(0))))
            return false;
    }

    // Possibly not needed.
    QCoreApplication::sendPostedEvents(this, QEvent::MetaCall);

    return m_state == QLlcpSocket::BoundState;
}

void QLlcpSocketPrivate::AccessFailed(const QDBusObjectPath &targetPath, const QString &kind,
                                      const QString &error)
{
    Q_UNUSED(targetPath);
    Q_UNUSED(kind);
    Q_UNUSED(error);

    Q_Q(QLlcpSocket);

    setSocketError(QLlcpSocket::SocketAccessError);

    m_state = QLlcpSocket::UnconnectedState;
    emit q->stateChanged(m_state);
}

void QLlcpSocketPrivate::AccessGranted(const QDBusObjectPath &targetPath,
                                       const QString &accessKind)
{
    Q_UNUSED(targetPath);
    Q_UNUSED(accessKind);
}

void QLlcpSocketPrivate::Accept(const QDBusVariant &lsap, const QDBusVariant &rsap,
                                int fd, const QVariantMap &properties)
{
    Q_UNUSED(lsap);
    Q_UNUSED(rsap);
    Q_UNUSED(fd);
    Q_UNUSED(properties);
}

void QLlcpSocketPrivate::Connect(const QDBusVariant &lsap, const QDBusVariant &rsap,
                                 int fd, const QVariantMap &properties)
{
    Q_UNUSED(lsap);
    Q_UNUSED(rsap);

    m_fd = fd;

    m_readNotifier = new QSocketNotifier(m_fd, QSocketNotifier::Read, this);
    connect(m_readNotifier, SIGNAL(activated(int)), this, SLOT(_q_readNotify()));
    m_writeNotifier = new QSocketNotifier(m_fd, QSocketNotifier::Write, this);
    connect(m_writeNotifier, SIGNAL(activated(int)), this, SLOT(_q_bytesWritten()));

    m_properties = properties;

    Q_Q(QLlcpSocket);

    q->setOpenMode(QIODevice::ReadWrite);

    m_state = QLlcpSocket::ConnectedState;
    emit q->stateChanged(m_state);
    emit q->connected();
}

void QLlcpSocketPrivate::Socket(const QDBusVariant &lsap, int fd, const QVariantMap &properties)
{
    m_fd = fd;
    m_port = lsap.variant().toUInt();

    m_readNotifier = new QSocketNotifier(m_fd, QSocketNotifier::Read, this);
    connect(m_readNotifier, SIGNAL(activated(int)), this, SLOT(_q_readNotify()));
    m_writeNotifier = new QSocketNotifier(m_fd, QSocketNotifier::Write, this);
    connect(m_writeNotifier, SIGNAL(activated(int)), this, SLOT(_q_bytesWritten()));

    m_properties = properties;

    Q_Q(QLlcpSocket);

    m_state = QLlcpSocket::BoundState;
    emit q->stateChanged(m_state);
}

void QLlcpSocketPrivate::_q_readNotify()
{
    Q_Q(QLlcpSocket);

    QByteArray datagram;
    datagram.resize(m_properties.value(QLatin1String("LocalMIU"), 128).toUInt());

    int readFromDevice = ::read(m_fd, datagram.data(), datagram.size());
    if (readFromDevice <= 0) {
        m_readNotifier->setEnabled(false);

        setSocketError(QLlcpSocket::RemoteHostClosedError);

        q->disconnectFromService();
        q->setOpenMode(QIODevice::NotOpen);
    } else {
        m_receivedDatagrams.append(datagram.left(readFromDevice));
        emit q->readyRead();
    }
}

void QLlcpSocketPrivate::_q_bytesWritten()
{
    Q_Q(QLlcpSocket);

    m_writeNotifier->setEnabled(false);

    emit q->bytesWritten(m_pendingBytes);
    m_pendingBytes = 0;
}

void QLlcpSocketPrivate::setSocketError(QLlcpSocket::SocketError socketError)
{
    Q_Q(QLlcpSocket);

    QLatin1String c("QLlcpSocket");

    m_error = socketError;
    switch (socketError) {
    case QLlcpSocket::UnknownSocketError:
        q->setErrorString(QLlcpSocket::tr("%1: Unknown error %2").arg(c).arg(errno));
        break;
    case QLlcpSocket::RemoteHostClosedError:
        q->setErrorString(QLlcpSocket::tr("%1: Remote closed").arg(c));
        break;
    case QLlcpSocket::SocketAccessError:
        q->setErrorString(QLlcpSocket::tr("%1: Socket access error"));
        break;
    case QLlcpSocket::SocketResourceError:
        q->setErrorString(QLlcpSocket::tr("%1: Socket resource error"));
        break;
    }

    emit q->error(m_error);
}

void QLlcpSocketPrivate::initializeRequestor()
{
    if (m_socketRequestor)
        return;

    if (m_requestorPath.isEmpty()) {
        m_requestorPath = QLatin1String(requestorBasePath) +
                          QString::number(requestorId.fetchAndAddOrdered(1));
    }

    Manager manager(QLatin1String("com.nokia.nfc"), QLatin1String("/"), m_connection);
    QDBusObjectPath defaultAdapterPath = manager.DefaultAdapter();

    if (!m_socketRequestor) {
        m_socketRequestor = new SocketRequestor(defaultAdapterPath.path(), this);

        connect(m_socketRequestor, SIGNAL(accessFailed(QDBusObjectPath,QString,QString)),
                this, SLOT(AccessFailed(QDBusObjectPath,QString,QString)));
        connect(m_socketRequestor, SIGNAL(accessGranted(QDBusObjectPath,QString)),
                this, SLOT(AccessGranted(QDBusObjectPath,QString)));
        connect(m_socketRequestor, SIGNAL(accept(QDBusVariant,QDBusVariant,int,QVariantMap)),
                this, SLOT(Accept(QDBusVariant,QDBusVariant,int,QVariantMap)));
        connect(m_socketRequestor, SIGNAL(connect(QDBusVariant,QDBusVariant,int,QVariantMap)),
                this, SLOT(Connect(QDBusVariant,QDBusVariant,int,QVariantMap)));
        connect(m_socketRequestor, SIGNAL(socket(QDBusVariant,int,QVariantMap)),
                this, SLOT(Socket(QDBusVariant,int,QVariantMap)));
    }
}

#include "moc_qllcpsocket_maemo6_p.cpp"

QTNFC_END_NAMESPACE
