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

#ifndef INPUTSTREAMTHREAD_H
#define INPUTSTREAMTHREAD_H

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

#include <QtCore/QObject>
#include <QtCore/QMutex>
#include <QtCore/QJniObject>
#include <jni.h>

QT_BEGIN_NAMESPACE

class QBluetoothSocketPrivateAndroid;

class InputStreamThread : public QObject
{
    Q_OBJECT
public:
    explicit InputStreamThread(QBluetoothSocketPrivateAndroid *socket_p);

    qint64 bytesAvailable() const;
    bool canReadLine() const;
    bool run();

    qint64 readData(char *data, qint64 maxSize);
    void javaThreadErrorOccurred(int errorCode);
    void javaReadyRead(jbyteArray buffer, int bufferLength);

    void prepareForClosure();

signals:
    void dataAvailable();
    void errorOccurred(int errorCode);

private:
    QBluetoothSocketPrivateAndroid *m_socket_p;
    QJniObject javaInputStreamThread;
    mutable QMutex m_mutex;
    bool expectClosure;
};

QT_END_NAMESPACE

#endif // INPUTSTREAMTHREAD_H
