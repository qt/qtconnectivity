/****************************************************************************
**
** Copyright (C) 2016 Lauri Laanmets (Proekspert AS) <lauri.laanmets@eesti.ee>
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
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
******************************************************************************/

#include <QtCore/QLoggingCategory>
#include <QtCore/QJniEnvironment>

#include "android/inputstreamthread_p.h"
#include "qbluetoothsocket_android_p.h"

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_ANDROID)

InputStreamThread::InputStreamThread(QBluetoothSocketPrivateAndroid *socket)
    : QObject(), m_socket_p(socket), expectClosure(false)
{
}

bool InputStreamThread::run()
{
    QMutexLocker lock(&m_mutex);

    javaInputStreamThread = QJniObject("org/qtproject/qt/android/bluetooth/QtBluetoothInputStreamThread");
    if (!javaInputStreamThread.isValid() || !m_socket_p->inputStream.isValid())
        return false;

    javaInputStreamThread.callMethod<void>("setInputStream", "(Ljava/io/InputStream;)V",
                                           m_socket_p->inputStream.object<jobject>());
    javaInputStreamThread.setField<jlong>("qtObject", reinterpret_cast<long>(this));
    javaInputStreamThread.setField<jboolean>("logEnabled", QT_BT_ANDROID().isDebugEnabled());

    javaInputStreamThread.callMethod<void>("start");

    return true;
}

qint64 InputStreamThread::bytesAvailable() const
{
    QMutexLocker locker(&m_mutex);
    return m_socket_p->rxBuffer.size();
}

bool InputStreamThread::canReadLine() const
{
    QMutexLocker locker(&m_mutex);
    return m_socket_p->rxBuffer.canReadLine();
}

qint64 InputStreamThread::readData(char *data, qint64 maxSize)
{
    QMutexLocker locker(&m_mutex);

    if (!m_socket_p->rxBuffer.isEmpty())
        return m_socket_p->rxBuffer.read(data, maxSize);

    return 0;
}

//inside the java thread
void InputStreamThread::javaThreadErrorOccurred(int errorCode)
{
    QMutexLocker lock(&m_mutex);

    if (!expectClosure)
        emit errorOccurred(errorCode);
    else
        emit errorOccurred(-1); // magic error, -1 means error was expected due to expected close()
}

//inside the java thread
void InputStreamThread::javaReadyRead(jbyteArray buffer, int bufferLength)
{
    QJniEnvironment env;

    QMutexLocker lock(&m_mutex);
    char *writePtr = m_socket_p->rxBuffer.reserve(bufferLength);
    env->GetByteArrayRegion(buffer, 0, bufferLength, reinterpret_cast<jbyte*>(writePtr));
    emit dataAvailable();
}

void InputStreamThread::prepareForClosure()
{
    QMutexLocker lock(&m_mutex);
    expectClosure = true;
}

QT_END_NAMESPACE
