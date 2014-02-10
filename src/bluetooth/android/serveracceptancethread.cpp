/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
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

#include <QtCore/QLoggingCategory>
#include <QtAndroidExtras/QAndroidJniEnvironment>

#include "android/serveracceptancethread_p.h"

Q_DECLARE_LOGGING_CATEGORY(QT_BT_ANDROID)

ServerAcceptanceThread::ServerAcceptanceThread(QObject *parent) :
    QThread(parent), m_stop(false), maxPendingConnections(1)
{
    btAdapter = QAndroidJniObject::callStaticObjectMethod("android/bluetooth/BluetoothAdapter",
                                                          "getDefaultAdapter",
                                                          "()Landroid/bluetooth/BluetoothAdapter;");
}

ServerAcceptanceThread::~ServerAcceptanceThread()
{
    Q_ASSERT(!isRunning());
    QMutexLocker lock(&m_mutex);
    shutdownPendingConnections();
}

void ServerAcceptanceThread::setServiceDetails(const QBluetoothUuid &uuid,
                                               const QString &serviceName,
                                               QBluetooth::SecurityFlags securityFlags)
{
    QMutexLocker lock(&m_mutex);
    m_uuid = uuid;
    m_serviceName = serviceName;
    secFlags = securityFlags;
}

void ServerAcceptanceThread::run()
{
    m_mutex.lock();

    qCDebug(QT_BT_ANDROID) << "Starting ServerSocketAccept thread";
    if (!validSetup()) {
        qCWarning(QT_BT_ANDROID) << "Invalid Server Socket setup";
        m_mutex.unlock();
        return;
    }

    shutdownPendingConnections();

    m_stop = false;

    QString tempUuid = m_uuid.toString();
    tempUuid.chop(1); //remove trailing '}'
    tempUuid.remove(0,1); //remove first '{'

    QAndroidJniEnvironment env;
    QAndroidJniObject inputString = QAndroidJniObject::fromString(tempUuid);
    QAndroidJniObject uuidObject = QAndroidJniObject::callStaticObjectMethod(
                "java/util/UUID", "fromString",
                "(Ljava/lang/String;)Ljava/util/UUID;",
                inputString.object<jstring>());
    inputString = QAndroidJniObject::fromString(m_serviceName);
    if (((int)secFlags) == 0) { //no form of security flag set
        qCDebug(QT_BT_ANDROID) << "InSecure listening";
        btServerSocket = btAdapter.callObjectMethod("listenUsingInsecureRfcommWithServiceRecord",
                                                "(Ljava/lang/String;Ljava/util/UUID;)Landroid/bluetooth/BluetoothServerSocket;",
                                                inputString.object<jstring>(),
                                                uuidObject.object<jobject>());
    } else {
        qCDebug(QT_BT_ANDROID) << "Secure listening";
        btServerSocket = btAdapter.callObjectMethod("listenUsingRfcommWithServiceRecord",
                                                    "(Ljava/lang/String;Ljava/util/UUID;)Landroid/bluetooth/BluetoothServerSocket;",
                                                    inputString.object<jstring>(),
                                                    uuidObject.object<jobject>());
    }
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        qCWarning(QT_BT_ANDROID) << "Cannot setup rfcomm socket listener";
        m_mutex.unlock();
        return;
    }

    if (!btServerSocket.isValid()) {
        qCWarning(QT_BT_ANDROID) << "Invalid BluetoothServerSocket";
        m_mutex.unlock();
        return;
    }

    while (!m_stop) {
        m_mutex.unlock();

        qCDebug(QT_BT_ANDROID) << "Waiting for new incoming socket";
        QAndroidJniEnvironment env;

        //this call blocks until we see an incoming connection
        QAndroidJniObject socket = btServerSocket.callObjectMethod("accept",
                                                                   "()Landroid/bluetooth/BluetoothSocket;");

        qCDebug(QT_BT_ANDROID) << "New socket accepted: ->" << socket.isValid();
        bool exceptionOccurred = false;
        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
            exceptionOccurred = true;
        }

        m_mutex.lock();

        if (exceptionOccurred || m_stop) {
            //if m_stop is true there is nothing really to be done but exit
            m_stop = true;
        } else if (socket.isValid()){
            if (pendingSockets.count() < maxPendingConnections) {
                pendingSockets.append(socket);
                emit newConnection();
            } else {
                qCWarning(QT_BT_ANDROID) << "Refusing connection due to limited pending socket queue";
                socket.callMethod<void>("close");
                if (env->ExceptionCheck()) {
                    qCWarning(QT_BT_ANDROID) << "Error during refusal of new socket";
                    env->ExceptionDescribe();
                    env->ExceptionClear();
                }


            }
        } else {
            //should never happen as invalid socket should cause exception
            qCWarning(QT_BT_ANDROID) << "Invalid state during server socket accept";
        }
    }

    m_uuid = QBluetoothUuid();
    m_serviceName = QString();
    btServerSocket = QAndroidJniObject();
    m_mutex.unlock();
}

void ServerAcceptanceThread::stop()
{
    QMutexLocker lock(&m_mutex);
    m_stop = true;

    QAndroidJniEnvironment env;
    if (btServerSocket.isValid()) {
        qCDebug(QT_BT_ANDROID) << "Closing server socket";
        btServerSocket.callMethod<void>("close");
        if (env->ExceptionCheck()) {
            qCWarning(QT_BT_ANDROID) << "Exception during closure of server socket";
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
        qCDebug(QT_BT_ANDROID) << "Closing server socket111";
    }
}

bool ServerAcceptanceThread::hasPendingConnections() const
{
    QMutexLocker lock(&m_mutex);
    return (pendingSockets.count() > 0);
}

/*
 * Returns the next pending connection or an invalid JNI object.
 * Note that even a stopped thread may still have pending
 * connections. Pending connections are only terminated upon
 * thread restart or destruction.
 */
QAndroidJniObject ServerAcceptanceThread::nextPendingConnection()
{
    QMutexLocker lock(&m_mutex);
    if (pendingSockets.isEmpty())
        return QAndroidJniObject();
    else
        return pendingSockets.takeFirst();
}

void ServerAcceptanceThread::setMaxPendingConnections(int maximumCount)
{
    QMutexLocker lock(&m_mutex);
    maxPendingConnections = maximumCount;
}

//must be run inside the lock but doesn't lock by itself
bool ServerAcceptanceThread::validSetup() const
{
    return (!m_uuid.isNull() && !m_serviceName.isEmpty());
}

//must be run inside the lock but doesn't lock by itself
void ServerAcceptanceThread::shutdownPendingConnections()
{
    while (!pendingSockets.isEmpty()) {
        QAndroidJniObject socket = pendingSockets.takeFirst();
        socket.callMethod<void>("close");
    }
}
