/****************************************************************************
**
** Copyright (C) 2016 Centria research and development
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtNfc module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
****************************************************************************/

#ifndef QLLCPSOCKET_ANDROID_P_H
#define QLLCPSOCKET_ANDROID_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qllcpsocket_p.h"

//#include "qnearfieldtarget_ANDROID_p.h"

QT_BEGIN_NAMESPACE

class QLlcpSocketPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(QLlcpSocket)

public:
    QLlcpSocketPrivate(QLlcpSocket *q);

    ~QLlcpSocketPrivate();

    void connectToService(QNearFieldTarget *target, const QString &serviceUri);

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

    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);

    qint64 bytesAvailable() const;
    bool canReadLine() const;

    bool waitForReadyRead(int msecs);
    bool waitForBytesWritten(int msecs);
    bool waitForConnected(int msecs);
    bool waitForDisconnected(int msecs);

    //Q_INVOKABLE void connected(nfc_target_t *);
    Q_INVOKABLE void targetLost();

    void dataRead(QByteArray&);
    void dataWritten();

public Q_SLOTS:
    void disconnectFromService();

private:
    QLlcpSocket *q_ptr;
    unsigned int m_sap;
    //nfc_llcp_connection_listener_t m_conListener;
    //NearFieldTarget *m_target;
    //nfc_target_t *m_target;

    QLlcpSocket::SocketState m_state;

    QList<QByteArray> m_receivedDatagrams;
    QList<QByteArray> m_writeQueue;

    bool m_server;

    enum llcpState {
        Idle, Reading, Writing
    } socketState;

private Q_SLOTS:
    void read();
    void enteringIdle();
};

QT_END_NAMESPACE

#endif // QLLCPSOCKET_ANDROID_P_H
