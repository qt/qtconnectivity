/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia. For licensing terms and
** conditions see http://qt.digia.com/licensing. For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights. These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qbluetoothservicediscoveryagent.h"
#include "qbluetoothtransferreply_osx_p.h"
#include "osx/osxbtobexsession_p.h"
#include "qbluetoothserviceinfo.h"
#include "osx/osxbtutility_p.h"
#include "qbluetoothuuid.h"


#include <QtCore/qcoreapplication.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/qtemporaryfile.h>
#include <QtCore/qmetaobject.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qstring.h>
#include <QtCore/qdebug.h>
#include <QtCore/qfile.h>
#include <QtCore/qdir.h>

QT_BEGIN_NAMESPACE

class QBluetoothTransferReplyOSXPrivate : OSXBluetooth::OBEXSessionDelegate
{
    friend class QBluetoothTransferReplyOSX;
public:
    QBluetoothTransferReplyOSXPrivate(QBluetoothTransferReplyOSX *q, QIODevice *inputStream);

    ~QBluetoothTransferReplyOSXPrivate();

    bool isActive() const;
    bool startOPP(const QBluetoothAddress &device);

    //
    void sendConnect(const QBluetoothAddress &device, quint16 channelID);
    void sendPut();

private:
    // OBEX session delegate:
    void OBEXConnectError(OBEXError errorCode, OBEXOpCode response) Q_DECL_OVERRIDE;
    void OBEXConnectSuccess() Q_DECL_OVERRIDE;

    void OBEXAbortSuccess() Q_DECL_OVERRIDE;

    void OBEXPutDataSent(quint32 current, quint32 total) Q_DECL_OVERRIDE;
    void OBEXPutSuccess() Q_DECL_OVERRIDE;
    void OBEXPutError(OBEXError error, OBEXOpCode response) Q_DECL_OVERRIDE;

    QBluetoothTransferReplyOSX *q_ptr;

    QIODevice *inputStream;

    QBluetoothTransferReply::TransferError error;
    QString errorString;

    // Set requestComplete, error, description, emit error, emit finished.
    // Too many things in one, but not to repeat this code everywhere.
    void setReplyError(QBluetoothTransferReply::TransferError errorCode,
                       const QString &errorMessage);

    // With a given API, we have to discover a service first
    // since we need a channel ID to work with OBEX session.
    // Also, service discovery agent does not have an interface
    // to test discovery mode, that's why we have this bool here.
    bool minimalScan;
    QScopedPointer<QBluetoothServiceDiscoveryAgent> agent;

    // The next step is to create an OBEX session:
    typedef OSXBluetooth::ObjCScopedPointer<ObjCOBEXSession> OBEXSession;
    OBEXSession session;

    // Both success and failure to send - transfer is complete.
    bool requestComplete;

    // We need a temporary file to generate an unique name
    // in case inputStream is not a file. QTemporaryFile not
    // only creates a random name, it also guarantees this name
    // is unique. The amount of code to generate such a name
    // is amaizingly huge (and will require global variables)
    // - so a temporary file can help.
    QScopedPointer<QTemporaryFile> temporaryFile;
};

QBluetoothTransferReplyOSXPrivate::QBluetoothTransferReplyOSXPrivate(QBluetoothTransferReplyOSX *q,
                                                                     QIODevice *input)
    : q_ptr(q),
      inputStream(input),
      error(QBluetoothTransferReply::NoError),
      minimalScan(true),
      requestComplete(false)
{
    Q_ASSERT_X(q, "QBluetoothTransferReplyOSXPrivate", "invalid q_ptr (null)");
}

QBluetoothTransferReplyOSXPrivate::~QBluetoothTransferReplyOSXPrivate()
{
    // closeSession will set a delegate to null.
    // The OBEX session will be closed then. If
    // somehow IOBluetooth/OBEX still has a reference to our
    // session, it will not call any of delegate's callbacks.
    if (session)
        [session closeSession];
}

bool QBluetoothTransferReplyOSXPrivate::isActive() const
{
    return agent || (session && [session hasActiveRequest]);
}

bool QBluetoothTransferReplyOSXPrivate::startOPP(const QBluetoothAddress &device)
{
    Q_ASSERT_X(!isActive(), "startOPP", "already started");
    Q_ASSERT_X(!device.isNull(), "startOPP", "invalid device address");

    errorString.clear();
    error = QBluetoothTransferReply::NoError;

    agent.reset(new QBluetoothServiceDiscoveryAgent);

    agent->setRemoteAddress(device);
    agent->setUuidFilter(QBluetoothUuid(QBluetoothUuid::ObexObjectPush));

    QObject::connect(agent.data(), SIGNAL(finished()), q_ptr, SLOT(serviceDiscoveryFinished()));
    QObject::connect(agent.data(), SIGNAL(error(QBluetoothServiceDiscoveryAgent::Error)),
                     q_ptr, SLOT(serviceDiscoveryError(QBluetoothServiceDiscoveryAgent::Error)));

    minimalScan = true;
    agent->start(QBluetoothServiceDiscoveryAgent::MinimalDiscovery);

    // We probably failed already.
    return error == QBluetoothTransferReply::NoError;
}

void QBluetoothTransferReplyOSXPrivate::sendConnect(const QBluetoothAddress &device, quint16 channelID)
{
    Q_ASSERT_X(!session, "sendConnect", "session is already active");

    error = QBluetoothTransferReply::NoError;
    errorString.clear();

    if (device.isNull() || !channelID) {
        qCWarning(QT_BT_OSX) << "QBluetoothTransferReplyOSXPrivate::sendConnect(), "
                                "invalid device address or port";

        setReplyError(QBluetoothTransferReply::HostNotFoundError,
                      QObject::tr("Invalid target address"));
        return;
    }

    OBEXSession newSession([[ObjCOBEXSession alloc] initWithDelegate:this
                            remoteDevice:device channelID:channelID]);
    if (!newSession) {
        qCWarning(QT_BT_OSX) << "QBluetoothTransferReplyOSXPrivate::sendConnect(), "
                                "failed to allocate OSXBTOBEXSession object";

        setReplyError(QBluetoothTransferReply::UnknownError,
                      QObject::tr("Failed to create an OBEX session"));
        return;
    }

    const OBEXError status = [newSession OBEXConnect];

    if ((status == kOBEXSuccess || status == kOBEXSessionAlreadyConnectedError)
        && error == QBluetoothTransferReply::NoError) {
        session.reset(newSession.take());
        if ([session isConnected])
            sendPut();// Connected, send a PUT request.
    } else {
        qCWarning(QT_BT_OSX) << "QBluetoothTransferReplyOSXPrivate::sendConnect(), "
                                "OBEXConnect failed";

        if (error == QBluetoothTransferReply::NoError) {
            // The error is not set yet.
            error = QBluetoothTransferReply::SessionError;
            errorString = QObject::tr("OBEXConnect failed");
        }

        requestComplete = true;
        emit q_ptr->error(error);
        emit q_ptr->finished(q_ptr);
    }
}

void QBluetoothTransferReplyOSXPrivate::sendPut()
{
    Q_ASSERT_X(inputStream, "sendPut", "invalid input stream (null)");
    Q_ASSERT_X(session, "sendPut", "invalid OBEX session (nil)");
    Q_ASSERT_X([session isConnected], "sendPut", "not connected");
    Q_ASSERT_X(![session hasActiveRequest], "sendPut",
               "session already has an active request");

    QString fileName;
    QFile *const file = qobject_cast<QFile *>(inputStream);
    if (file) {
        if (!file->exists()) {
            setReplyError(QBluetoothTransferReply::FileNotFoundError,
                          QObject::tr("Source file does not exist"));
            return;
        } else if (!file->isReadable()) {
            file->open(QIODevice::ReadOnly);
            if (!file->isReadable()) {
                setReplyError(QBluetoothTransferReply::IODeviceNotReadableError,
                              QObject::tr("File cannot be read. "
                                          "Make sure it is open for reading."));
                return;
            }
        }

        fileName = file->fileName();
    } else {
        if (!inputStream->isReadable()) {
            setReplyError(QBluetoothTransferReply::IODeviceNotReadableError,
                          QObject::tr("QIODevice cannot be read. "
                                      "Make sure it is open for reading."));
            return;
        }

        temporaryFile.reset(new QTemporaryFile);
        temporaryFile->open();
        fileName = temporaryFile->fileName();
    }

    const QFileInfo fileInfo(fileName);
    fileName = fileInfo.fileName();

    if ([session OBEXPutFile:inputStream withName:fileName] != kOBEXSuccess) {
        // TODO: convert OBEXError into something reasonable?
        setReplyError(QBluetoothTransferReply::SessionError,
                      QObject::tr("OBEX put failed"));
    }
}


void QBluetoothTransferReplyOSXPrivate::OBEXConnectError(OBEXError errorCode, OBEXOpCode response)
{
    Q_UNUSED(errorCode)
    Q_UNUSED(response)

    if (session) {
        setReplyError(QBluetoothTransferReply::SessionError,
                      QObject::tr("OBEX connect failed"));
    } else {
        // Else we're still in OBEXConnect, in a call-back
        // and do not want to emit yet (will be done a bit later).
        error = QBluetoothTransferReply::SessionError;
        errorString = QObject::tr("OBEX connect failed");
        requestComplete = true;
    }
}

void QBluetoothTransferReplyOSXPrivate::OBEXConnectSuccess()
{
    // Now that OBEX connect succeeded, we can send an OBEX put request.
    if (!session) {
        // We're still in OBEXConnect(), it'll take care of next steps.
        return;
    }

    sendPut();
}

void QBluetoothTransferReplyOSXPrivate::OBEXAbortSuccess()
{
    // TODO:
}

void QBluetoothTransferReplyOSXPrivate::OBEXPutDataSent(quint32 current, quint32 total)
{
    emit q_ptr->transferProgress(current, total);
}

void QBluetoothTransferReplyOSXPrivate::OBEXPutSuccess()
{
    requestComplete = true;
    emit q_ptr->finished(q_ptr);
}

void QBluetoothTransferReplyOSXPrivate::OBEXPutError(OBEXError errorCode, OBEXOpCode responseCode)
{
    // Error can be reported by errorCode or responseCode
    // (that's how errors are reported in OBEXSession events).
    // errorCode and responseCode are "mutually exclusive".

    Q_UNUSED(responseCode)

    if (errorCode != kOBEXSuccess) {
        // TODO: errorCode -> TransferError.
    } else {
        // TODO: a response code can give some interesting information,
        // like "forbidden" etc. - convert this into more reasonable error.
    }

    setReplyError(QBluetoothTransferReply::SessionError,
                  QObject::tr("OBEX PUT command failed"));
}

void QBluetoothTransferReplyOSXPrivate::setReplyError(QBluetoothTransferReply::TransferError errorCode,
                                                      const QString &description)
{
    // Not to be used to clear an error!

    error = errorCode;
    errorString = description;
    requestComplete = true;
    emit q_ptr->error(error);
    emit q_ptr->finished(q_ptr);
}


QBluetoothTransferReplyOSX::QBluetoothTransferReplyOSX(QIODevice *input,
                                                       const QBluetoothTransferRequest &request,
                                                       QBluetoothTransferManager *manager)
                                : QBluetoothTransferReply(manager)

{
    Q_UNUSED(input)

    setManager(manager);
    setRequest(request);

    osx_d_ptr.reset(new QBluetoothTransferReplyOSXPrivate(this, input));

    if (input) {
        QMetaObject::invokeMethod(this, "start", Qt::QueuedConnection);
    } else {
        qCWarning(QT_BT_OSX) << "QBluetoothTransferReplyOSX::QBluetoothTransferReplyOSX(), "
                                "invalid input stream (null)";

        osx_d_ptr->requestComplete = true;
        osx_d_ptr->errorString = tr("Invalid input file (null)");
        osx_d_ptr->error = FileNotFoundError;
        QMetaObject::invokeMethod(this, "error", Qt::QueuedConnection,
                                  Q_ARG(QBluetoothTransferReply::TransferError, FileNotFoundError));
    }
}

QBluetoothTransferReplyOSX::~QBluetoothTransferReplyOSX()
{
    // A dtor to make a scoped pointer with incomplete type happy.
}

QBluetoothTransferReply::TransferError QBluetoothTransferReplyOSX::error() const
{
    return osx_d_ptr->error;
}

QString QBluetoothTransferReplyOSX::errorString() const
{
    return osx_d_ptr->errorString;
}

bool QBluetoothTransferReplyOSX::isFinished() const
{
    return osx_d_ptr->requestComplete;
}

bool QBluetoothTransferReplyOSX::isRunning() const
{
    return osx_d_ptr->isActive();
}

bool QBluetoothTransferReplyOSX::abort()
{
    // Reset a delegate.
    [osx_d_ptr->session closeSession];
    // Should never be called from an OBEX callback!
    osx_d_ptr->session.reset(Q_NULLPTR);

    // Not setReplyError, we emit finished only!
    osx_d_ptr->requestComplete = true;
    osx_d_ptr->errorString = tr("Operation canceled");
    osx_d_ptr->error = UserCanceledTransferError;

    emit finished(this);

    return true;
}

bool QBluetoothTransferReplyOSX::start()
{
    // OBEXSession requires a channel ID and we have to find it first,
    // using QBluetoothServiceDiscoveryAgent (singleDevice, OBEX uuid filter, start
    // from MinimalDiscovery mode and continue with FullDiscovery if
    // MinimalDiscovery fails.

    if (!osx_d_ptr->isActive()) {
        // Step 0: find a channelID.
        if (request().address().isNull()) {
            qCWarning(QT_BT_OSX) << "QBluetoothTransferReplyOSX::start(), "
                                    "invalid device address";
            osx_d_ptr->setReplyError(HostNotFoundError, tr("Invalid target address"));
            return false;
        }

        return osx_d_ptr->startOPP(request().address());
    } else {
        osx_d_ptr->setReplyError(UnknownError, tr("Transfer already started"));
        return false;
    }
}

void QBluetoothTransferReplyOSX::serviceDiscoveryFinished()
{
    Q_ASSERT_X(osx_d_ptr->agent.data(), "serviceDiscoveryFinished",
               "invalid service discovery agent (null)");

    const QList<QBluetoothServiceInfo> services = osx_d_ptr->agent->discoveredServices();
    if (services.size()) {
        // TODO: what if we have several?
        const QBluetoothServiceInfo &foundOBEX = services.front();
        osx_d_ptr->sendConnect(request().address(), foundOBEX.serverChannel());
    } else {
        if (osx_d_ptr->minimalScan) {
            // Try full discovery now.
            osx_d_ptr->minimalScan = false;
            osx_d_ptr->agent->start(QBluetoothServiceDiscoveryAgent::FullDiscovery);
        } else {
            // No service record, no channel ID, no OBEX session.
            osx_d_ptr->setReplyError(HostNotFoundError, tr("OBEX/OPP serice not found"));
        }
    }
}

void QBluetoothTransferReplyOSX::serviceDiscoveryError(QBluetoothServiceDiscoveryAgent::Error errorCode)
{
    Q_ASSERT_X(osx_d_ptr->agent.data(), "serviceDiscoveryError", "invalid service discovery agent (null)");

    if (errorCode == QBluetoothServiceDiscoveryAgent::PoweredOffError) {
        // There's nothing else we can do.
        osx_d_ptr->setReplyError(UnknownError, tr("Bluetooth adapter is powered off"));
        return;
    }

    if (osx_d_ptr->minimalScan) {// Try again, this time in FullDiscovery mode.
        osx_d_ptr->minimalScan = false;
        osx_d_ptr->agent->start(QBluetoothServiceDiscoveryAgent::FullDiscovery);
    } else {
        osx_d_ptr->setReplyError(HostNotFoundError, tr("Invalid target address"));
    }
}

QT_END_NAMESPACE
