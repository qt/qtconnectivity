/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
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
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QLLCPSOCKET_MAEMO6_P_H
#define QLLCPSOCKET_MAEMO6_P_H

#include <qconnectivityglobal.h>

#include "qllcpsocket.h"

#include <QtDBus/QDBusConnection>

QT_FORWARD_DECLARE_CLASS(QDBusObjectPath)
QT_FORWARD_DECLARE_CLASS(QDBusVariant)
QT_FORWARD_DECLARE_CLASS(QSocketNotifier)

class AccessRequestorAdaptor;
class LLCPRequestorAdaptor;

QTNFC_BEGIN_NAMESPACE

class SocketRequestor;

class QLlcpSocketPrivate : public QObject
{
    Q_OBJECT

    Q_DECLARE_PUBLIC(QLlcpSocket)

public:
    QLlcpSocketPrivate(QLlcpSocket *q);
    QLlcpSocketPrivate(const QDBusConnection &connection, int fd, const QVariantMap &properties);
    ~QLlcpSocketPrivate();

    void connectToService(QNearFieldTarget *target, const QString &serviceUri);
    void disconnectFromService();

    bool bind(quint8 port);

    bool hasPendingDatagrams() const;
    qint64 pendingDatagramSize() const;

    qint64 writeDatagram(const char *data, qint64 size);
    qint64 writeDatagram(const QByteArray &datagram);

    qint64 readDatagram(char *data, qint64 maxSize,
                        QNearFieldTarget **target = 0, quint8 *port = 0);
    qint64 writeDatagram(const char *data, qint64 size,
                         QNearFieldTarget *target, quint8 port);
    qint64 writeDatagram(const QByteArray &datagram, QNearFieldTarget *target, quint8 port);

    QLlcpSocket::SocketError error() const;
    QLlcpSocket::SocketState state() const;

    qint64 readData(char *data, qint64 maxlex, quint8 *port = 0);
    qint64 writeData(const char *data, qint64 len);

    qint64 bytesAvailable() const;
    bool canReadLine() const;

    bool waitForReadyRead(int msecs);
    bool waitForBytesWritten(int msecs);
    bool waitForConnected(int msecs);
    bool waitForDisconnected(int msecs);
    bool waitForBound(int msecs);

private slots:
    // com.nokia.nfc.AccessRequestor
    void AccessFailed(const QDBusObjectPath &targetPath, const QString &kind,
                      const QString &error);
    void AccessGranted(const QDBusObjectPath &targetPath, const QString &accessKind);

    // com.nokia.nfc.LLCPRequestor
    void Accept(const QDBusVariant &lsap, const QDBusVariant &rsap, int fd, const QVariantMap &properties);
    void Connect(const QDBusVariant &lsap, const QDBusVariant &rsap, int fd, const QVariantMap &properties);
    void Socket(const QDBusVariant &lsap, int fd, const QVariantMap &properties);

    void _q_readNotify();
    void _q_bytesWritten();

private:
    void setSocketError(QLlcpSocket::SocketError socketError);
    void initializeRequestor();

    QLlcpSocket *q_ptr;
    QVariantMap m_properties;
    QList<QByteArray> m_receivedDatagrams;

    QDBusConnection m_connection;

    QString m_serviceUri;
    quint8 m_port;

    QString m_requestorPath;

    SocketRequestor *m_socketRequestor;

    int m_fd;
    QSocketNotifier *m_readNotifier;
    QSocketNotifier *m_writeNotifier;
    qint64 m_pendingBytes;

    QLlcpSocket::SocketState m_state;
    QLlcpSocket::SocketError m_error;
};

QTNFC_END_NAMESPACE

#endif // QLLCPSOCKET_MAEMO6_P_H
