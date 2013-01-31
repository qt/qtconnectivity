/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
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

#ifndef QLLCPSOCKET_H
#define QLLCPSOCKET_H

#include <QtCore/QIODevice>
#include <QtNetwork/QAbstractSocket>
#include <QtNfc/qnfcglobal.h>

QTNFC_BEGIN_NAMESPACE

class QNearFieldTarget;
class QLlcpSocketPrivate;

class Q_NFC_EXPORT QLlcpSocket : public QIODevice
{
    Q_OBJECT

    Q_DECLARE_PRIVATE(QLlcpSocket)

    friend class QLlcpServerPrivate;

public:
    enum SocketState {
        UnconnectedState = QAbstractSocket::UnconnectedState,
        ConnectingState = QAbstractSocket::ConnectingState,
        ConnectedState = QAbstractSocket::ConnectedState,
        ClosingState = QAbstractSocket::ClosingState,
        BoundState = QAbstractSocket::BoundState,
        ListeningState = QAbstractSocket::ListeningState
    };

    enum SocketError {
        UnknownSocketError = QAbstractSocket::UnknownSocketError,
        RemoteHostClosedError = QAbstractSocket::RemoteHostClosedError,
        SocketAccessError = QAbstractSocket::SocketAccessError,
        SocketResourceError = QAbstractSocket::SocketResourceError
    };

    explicit QLlcpSocket(QObject *parent = 0);
    ~QLlcpSocket();

    void connectToService(QNearFieldTarget *target, const QString &serviceUri);
    void disconnectFromService();

    void close();

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

    SocketError error() const;
    SocketState state() const;

    qint64 bytesAvailable() const;
    bool canReadLine() const;

    bool waitForReadyRead(int msecs = 30000);
    bool waitForBytesWritten(int msecs = 30000);
    virtual bool waitForConnected(int msecs = 30000);
    virtual bool waitForDisconnected(int msecs = 30000);
    bool isSequential() const;

Q_SIGNALS:
    void connected();
    void disconnected();
    void error(QLlcpSocket::SocketError socketError);
    void stateChanged(QLlcpSocket::SocketState socketState);

protected:
    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);

private:
    QLlcpSocket(QLlcpSocketPrivate *d, QObject *parent);

    QLlcpSocketPrivate *d_ptr;
};

QTNFC_END_NAMESPACE

#endif // QLLCPSOCKET_H
