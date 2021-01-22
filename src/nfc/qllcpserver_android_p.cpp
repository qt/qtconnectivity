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

#include "qllcpserver_android_p.h"
//#include "qnx/qnxnfcmanager_p.h"

QT_BEGIN_NAMESPACE

QLlcpServerPrivate::QLlcpServerPrivate(QLlcpServer *q)
    : q_ptr(q), m_llcpSocket(0), m_connected(false)
{
}

QLlcpServerPrivate::~QLlcpServerPrivate()
{
}

bool QLlcpServerPrivate::listen(const QString &/*serviceUri*/)
{
    /*//The server is already listening
    if (isListening())
        return false;

    nfc_result_t result = nfc_llcp_register_connection_listener(NFC_LLCP_SERVER, 0, serviceUri.toStdString().c_str(), &m_conListener);
    m_connected = true;
    if (result == NFC_RESULT_SUCCESS) {
        m_serviceUri = serviceUri;
        qQNXNFCDebug() << "LLCP server registered" << serviceUri;
    } else {
        qWarning() << Q_FUNC_INFO << "Could not register for llcp connection listener";
        return false;
    }
    QNXNFCManager::instance()->registerLLCPConnection(m_conListener, this);*/
    return true;
}

bool QLlcpServerPrivate::isListening() const
{
    return m_connected;
}

void QLlcpServerPrivate::close()
{
    /*nfc_llcp_unregister_connection_listener(m_conListener);
    QNXNFCManager::instance()->unregisterLLCPConnection(m_conListener);
    m_serviceUri = QString();
    m_connected = false;*/
}

QString QLlcpServerPrivate::serviceUri() const
{
    return m_serviceUri;
}

quint8 QLlcpServerPrivate::serverPort() const
{
    /*unsigned int sap;
    if (nfc_llcp_get_local_sap(m_target, &sap) == NFC_RESULT_SUCCESS) {
        return sap;
    }*/
    return -1;
}

bool QLlcpServerPrivate::hasPendingConnections() const
{
    return m_llcpSocket != 0;
}

QLlcpSocket *QLlcpServerPrivate::nextPendingConnection()
{
    /*QLlcpSocket *socket = m_llcpSocket;
    m_llcpSocket = 0;
    return socket;*/
    return 0;
}

QLlcpSocket::SocketError QLlcpServerPrivate::serverError() const
{
    return QLlcpSocket::UnknownSocketError;
}

/*void QLlcpServerPrivate::connected(nfc_target_t *target)
{
    m_target = target;
    if (m_llcpSocket != 0) {
        qWarning() << Q_FUNC_INFO << "LLCP socket not cloesed properly";
        return;
    }
    m_llcpSocket = new QLlcpSocket();
    m_llcpSocket->bind(serverPort());
}*/

QT_END_NAMESPACE


