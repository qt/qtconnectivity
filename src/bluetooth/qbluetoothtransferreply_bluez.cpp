/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
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


#include "qbluetoothtransferreply_bluez_p.h"
#include "qbluetoothaddress.h"

#include "bluez/obex_client_p.h"
#include "bluez/obex_manager_p.h"
#include "bluez/obex_agent_p.h"
#include "bluez/obex_transfer_p.h"
#include "qbluetoothtransferreply.h"

#include <QtCore/QLoggingCategory>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrentRun>

static const QLatin1String agentPath("/qt/agent");

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_BLUEZ)

QBluetoothTransferReplyBluez::QBluetoothTransferReplyBluez(QIODevice *input, const QBluetoothTransferRequest &request,
                                                           QBluetoothTransferManager *parent)
:   QBluetoothTransferReply(parent), tempfile(0), source(input),
    m_running(false), m_finished(false), m_size(0),
    m_error(QBluetoothTransferReply::NoError), m_errorStr(), m_transfer_path()
{
    setRequest(request);
    setManager(parent);
    client = new OrgOpenobexClientInterface(QLatin1String("org.openobex.client"), QLatin1String("/"),
                                           QDBusConnection::sessionBus());

    qsrand(QTime::currentTime().msec());
    m_agent_path = agentPath;
    m_agent_path.append(QString::fromLatin1("/%1").arg(qrand()));

    agent = new AgentAdaptor(this);

    bool res = QDBusConnection::sessionBus().registerObject(m_agent_path, this);
    if(!res)
        qCWarning(QT_BT_BLUEZ) << "Failed Creating dbus objects";

    qRegisterMetaType<QBluetoothTransferReply*>("QBluetoothTransferReply*");
    QMetaObject::invokeMethod(this, "start", Qt::QueuedConnection);
    m_running = true;
}

/*!
    Destroys the QBluetoothTransferReply object.
*/
QBluetoothTransferReplyBluez::~QBluetoothTransferReplyBluez()
{
    QDBusConnection::sessionBus().unregisterObject(m_agent_path);
    delete client;
}

bool QBluetoothTransferReplyBluez::start()
{
//    qDebug() << "Got a:" << source->metaObject()->className();
    QFile *file = qobject_cast<QFile *>(source);

    if(!file){
        tempfile = new QTemporaryFile(this );
        tempfile->open();
        qCDebug(QT_BT_BLUEZ) << "Not a QFile, making a copy" << tempfile->fileName();
        if (!source->isReadable()) {
            m_errorStr = QBluetoothTransferReply::tr("QIODevice cannot be read."
                                                     "Make sure it is open for reading.");
            m_error = QBluetoothTransferReply::IODeviceNotReadableError;
            m_finished = true;
            m_running = false;
            QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection, Q_ARG(QBluetoothTransferReply*, this));
            return false;
        }

        QFutureWatcher<bool> *watcher = new QFutureWatcher<bool>();
        QObject::connect(watcher, SIGNAL(finished()), this, SLOT(copyDone()));

        QFuture<bool> results = QtConcurrent::run(QBluetoothTransferReplyBluez::copyToTempFile, tempfile, source);
        watcher->setFuture(results);
    }
    else {
        if (!file->exists()) {
            m_errorStr = QBluetoothTransferReply::tr("Source file does not exist");
            m_error = QBluetoothTransferReply::FileNotFoundError;
            m_finished = true;
            m_running = false;
            QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection, Q_ARG(QBluetoothTransferReply*, this));
            return false;
        }
        if (request().address().isNull()) {
            m_errorStr = QBluetoothTransferReply::tr("Invalid target address");
            m_error = QBluetoothTransferReply::HostNotFoundError;
            m_finished = true;
            m_running = false;
            QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection, Q_ARG(QBluetoothTransferReply*, this));
            return false;
        }
        m_size = file->size();
        startOPP(file->fileName());
    }
    return true;
}

bool QBluetoothTransferReplyBluez::copyToTempFile(QIODevice *to, QIODevice *from)
{
    char *block = new char[4096];
    int size;

    while ((size = from->read(block, 4096)) > 0) {
        if(size != to->write(block, size)){
            return false;
        }
    }

    delete[] block;
    return true;
}

void QBluetoothTransferReplyBluez::copyDone()
{
    m_size = tempfile->size();
    startOPP(tempfile->fileName());
    QObject::sender()->deleteLater();
}

void QBluetoothTransferReplyBluez::startOPP(QString filename)
{
    QVariantMap device;
    QStringList files;

    device.insert(QString::fromLatin1("Destination"), request().address().toString());
    files << filename;

    QDBusObjectPath path(m_agent_path);
    QDBusPendingReply<> sendReply = client->SendFiles(device, files, path);

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(sendReply, this);
    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                     this, SLOT(sendReturned(QDBusPendingCallWatcher*)));
}

void QBluetoothTransferReplyBluez::sendReturned(QDBusPendingCallWatcher *watcher)
{
    QDBusPendingReply<> sendReply = *watcher;
    if(sendReply.isError()){
        m_finished = true;
        m_running = false;
        m_errorStr = sendReply.error().message();
        if (m_errorStr == QStringLiteral("Could not open file for sending")) {
            m_error = QBluetoothTransferReply::FileNotFoundError;
            m_errorStr = tr("Could not open file for sending");
        } else if (m_errorStr == QStringLiteral("The transfer was canceled")) {
            m_error = QBluetoothTransferReply::UserCanceledTransferError;
            m_errorStr = tr("The transfer was canceled");
        } else {
            m_error = QBluetoothTransferReply::UnknownError;
        }

        // allow time for the developer to connect to the signal
        QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection, Q_ARG(QBluetoothTransferReply*, this));
    }
}

QBluetoothTransferReply::TransferError QBluetoothTransferReplyBluez::error() const
{
    return m_error;
}

QString QBluetoothTransferReplyBluez::errorString() const
{
    return m_errorStr;
}

void QBluetoothTransferReplyBluez::Complete(const QDBusObjectPath &in0)
{
    Q_UNUSED(in0);
    m_transfer_path.clear();
    m_finished = true;
    m_running = false;
}

void QBluetoothTransferReplyBluez::Error(const QDBusObjectPath &in0, const QString &in1)
{
    Q_UNUSED(in0);
    m_transfer_path.clear();
    m_finished = true;
    m_running = false;
    m_errorStr = in1;
    if (in1 == QStringLiteral("Could not open file for sending")) {
        m_error = QBluetoothTransferReply::FileNotFoundError;
        m_errorStr = tr("Could not open file for sending");
    } else if (in1 == QStringLiteral("Operation canceled")) {
        m_error = QBluetoothTransferReply::UserCanceledTransferError;
        m_errorStr = tr("Operation canceled");
    } else {
        m_error = QBluetoothTransferReply::UnknownError;
    }

    emit finished(this);
}

void QBluetoothTransferReplyBluez::Progress(const QDBusObjectPath &in0, qulonglong in1)
{
    Q_UNUSED(in0);
    emit transferProgress(in1, m_size);
}

void QBluetoothTransferReplyBluez::Release()
{
    if(m_errorStr.isEmpty())
        emit finished(this);
}

QString QBluetoothTransferReplyBluez::Request(const QDBusObjectPath &in0)
{
    m_transfer_path = in0.path();

    return QString();
}

/*!
    Returns true if this reply has finished; otherwise returns false.
*/
bool QBluetoothTransferReplyBluez::isFinished() const
{
    return m_finished;
}

/*!
    Returns true if this reply is running; otherwise returns false.
*/
bool QBluetoothTransferReplyBluez::isRunning() const
{
    return m_running;
}

void QBluetoothTransferReplyBluez::abort()
{
    if(!m_transfer_path.isEmpty()){
        OrgOpenobexTransferInterface *xfer = new OrgOpenobexTransferInterface(QLatin1String("org.openobex.client"), m_transfer_path,
                                                                              QDBusConnection::sessionBus());
        QDBusPendingReply<> reply = xfer->Cancel();
        reply.waitForFinished();
        if(reply.isError()){
            qCWarning(QT_BT_BLUEZ) << "Failed to abort transfer" << reply.error();
        }
        delete xfer;
    }
}

#include "moc_qbluetoothtransferreply_bluez_p.cpp"

QT_END_NAMESPACE
