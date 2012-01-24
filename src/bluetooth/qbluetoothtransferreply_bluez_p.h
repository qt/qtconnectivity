/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
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

#ifndef QBLUETOOTHTRANSFERREPLY_BLUEZ_P_H
#define QBLUETOOTHTRANSFERREPLY_BLUEZ_P_H

#include <QtCore/QIODevice>
#include <QtDBus/QtDBus>

#include <qbluetoothtransferrequest.h>
#include <qbluetoothtransfermanager.h>

#include "qbluetoothtransferreply.h"

#ifdef NOKIA_BT_SERVICES
#include <QtServiceFramework/QServiceManager>
#endif

QT_BEGIN_HEADER

class OrgOpenobexClientInterface;
class OrgOpenobexManagerInterface;
class AgentAdaptor;

QTBLUETOOTH_BEGIN_NAMESPACE

class Q_BLUETOOTH_EXPORT QBluetoothTransferReplyBluez : public QBluetoothTransferReply
{
    Q_OBJECT

public:
    explicit QBluetoothTransferReplyBluez(QIODevice *input, QObject *parent = 0);
    ~QBluetoothTransferReplyBluez();

    QVariant attribute(QBluetoothTransferRequest::Attribute code) const;
    bool isFinished() const;
    bool isRunning() const;

    bool start();

    void startOPP(QString filename);

    void setAddress(const QBluetoothAddress &address);

    QBluetoothTransferReply::TransferError error() const;
    QString errorString() const;

protected:
    qint64 readData(char*, qint64);
    qint64 writeData(const char*, qint64);

private:
    OrgOpenobexClientInterface *client;
    OrgOpenobexManagerInterface *manager;
    AgentAdaptor *agent;

    QTemporaryFile *tempfile;
    QIODevice *source;

    bool m_running;
    bool m_finished;

    quint64 m_size;

    QBluetoothAddress address;

    QBluetoothTransferReply::TransferError m_error;
    QString m_errorStr;

    QString m_agent_path;

    QString m_transfer_path;

#ifdef NOKIA_BT_SERVICES
    static QObject *s_obexService;
#endif

    static bool copyToTempFile(QIODevice *to, QIODevice *from);

private slots:
    void copyDone();

public slots:
    void abort();
    void Complete(const QDBusObjectPath &in0);
    void Error(const QDBusObjectPath &in0, const QString &in1);
    void Progress(const QDBusObjectPath &in0, qulonglong in1);
    void Release();
    QString Request(const QDBusObjectPath &in0);
    void sendReturned(QDBusPendingCallWatcher*);

};

#ifdef NOKIA_BT_SERVICES
class NokiaBtServiceConnection: public QObject
{
    Q_OBJECT
public:
    NokiaBtServiceConnection();
    void acquire();
    void release();

    void outgoingFile(const QString &transferId, const QString &remoteDevice, const QString &fileName, const QString &mimeType, quint64 size);
    void setTransferStarted(const QString &transferId);
    void setTransferFinished(const QString &transferId, bool success);
    void setTranferProgress(const QString &transferId, quint64 progress, quint64 total);

private:
    QObject *m_obexService;
    int m_refCount;
    QMutex m_refCountMutex;

private slots:
    void connectToObexServerService();
    void disconnectFromObexServerService();
    void sfwIPCError(QService::UnrecoverableIPCError);
};
Q_GLOBAL_STATIC(NokiaBtServiceConnection, nokiaBtServiceInstance)

#endif

QTBLUETOOTH_END_NAMESPACE

QT_END_HEADER

#endif // QBLUETOOTHTRANSFERREPLY_H
