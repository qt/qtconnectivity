/***************************************************************************
**
** Copyright (C) 2013 Research In Motion
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


#include "qbluetoothtransferreply_qnx_p.h"
#include "qbluetoothaddress.h"

#include "qbluetoothtransferreply.h"

#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrentRun>

#include "qnx/ppshelpers_p.h"
#include <QSocketNotifier>

#include <QtCore/private/qcore_unix_p.h>

#include <QTime>

//static const QLatin1String agentPath("/shared/tmp/opp");

QT_BEGIN_NAMESPACE_BLUETOOTH

QBluetoothTransferReplyQnx::QBluetoothTransferReplyQnx(QIODevice *input, const QBluetoothTransferRequest &request,
                                                       QObject *parent)
:   QBluetoothTransferReply(parent), tempfile(0), source(input),
    m_running(false), m_finished(false),
    m_error(QBluetoothTransferReply::NoError), m_errorStr()
{
    ppsRegisterControl();
    //qsrand(QTime::currentTime().msec());
    //m_agent_path = agentPath;
    //m_agent_path.append(QString::fromLatin1("/%1").arg(qrand()));
    ppsRegisterForEvent("opp_update", this);
    ppsRegisterForEvent("opp_complete", this);
    ppsRegisterForEvent("opp_cancelled", this);

    QMetaObject::invokeMethod(this, "start", Qt::QueuedConnection);
}

/*!
    Destroys the QBluetoothTransferReply object.
*/
QBluetoothTransferReplyQnx::~QBluetoothTransferReplyQnx()
{
    ppsUnregisterControl(this);
}

bool QBluetoothTransferReplyQnx::start()
{
    m_running = true;
    m_error = QBluetoothTransferReply::NoError;
    m_errorStr = QString();

    QFile *file = qobject_cast<QFile *>(source);

    if (!file){
//        tempfile = new QTemporaryFile(this );
//        tempfile->open();

//        QFutureWatcher<bool> *watcher = new QFutureWatcher<bool>();
//        QObject::connect(watcher, SIGNAL(finished()), this, SLOT(copyDone()));

//        QFuture<bool> results = QtConcurrent::run(QBluetoothTransferReplyQnx::copyToTempFile, tempfile, source);
//        watcher->setFuture(results);
        //QTemporaryFile does not work properly yet
        m_error = QBluetoothTransferReply::UnknownError;
        m_finished = true;
        m_running = false;
        Q_EMIT finished(this);

    } else {
        startOPP(file->fileName());
    }
    return true;
}

bool QBluetoothTransferReplyQnx::copyToTempFile(QIODevice *to, QIODevice *from)
{
    char *block = new char[4096];
    int size;

    while ((size = from->read(block, 4096))) {
        if (size != to->write(block, size))
            return false;
    }

    delete[] block;
    return true;
}

void QBluetoothTransferReplyQnx::copyDone()
{
    qBBBluetoothDebug() << "Copy done";
    startOPP(tempfile->fileName());
    QObject::sender()->deleteLater();
}

void QBluetoothTransferReplyQnx::startOPP(QString filename)
{
    qBBBluetoothDebug() << "Sending Push object command";
    ppsSendOpp("push_object", filename.toUtf8(), m_address, this);
}

QBluetoothTransferReply::TransferError QBluetoothTransferReplyQnx::error() const
{
    return m_error;
}

QString QBluetoothTransferReplyQnx::errorString() const
{
    return m_errorStr;
}

void QBluetoothTransferReplyQnx::controlReply(ppsResult result)
{
    if (!result.errorMsg.isEmpty()) {
        m_errorStr = result.errorMsg;
        m_error = QBluetoothTransferReply::UnknownError;
    }
}

void QBluetoothTransferReplyQnx::controlEvent(ppsResult result)
{
    if (result.msg == QStringLiteral("opp_cancelled")) {
        qBBBluetoothDebug() << "opp cancelled" << result.errorMsg << result.error;
        if (m_running)
            return;
        m_finished = true;
        m_running = false;
//        bool ok;
//        int reason = result.dat.at(result.dat.indexOf(QStringLiteral("reason")) + 1).toInt(&ok);
//        if (ok) {
//            switch (reason) {
//            case 1: m_error = QBluetoothTransferReply::UserCanceledTransferError;
//            case 3: m_error = QBluetoothTransferReply::UserCanceledTransferError;
//            }
//        } else {
        m_errorStr = result.errorMsg;
        m_error = QBluetoothTransferReply::UnknownError;
//      }
        Q_EMIT finished(this);
    } else if (result.msg == QStringLiteral("opp_update")) {
        bool ok;
        int sentBytes = result.dat.at(result.dat.indexOf(QStringLiteral("sent")) + 1).toInt(&ok);
        if (!ok)
            return;
        int totalBytes = result.dat.at(result.dat.indexOf(QStringLiteral("total")) + 1).toInt(&ok);
        if (!ok)
            return;
        qBBBluetoothDebug() << "opp update" << sentBytes << totalBytes;
        Q_EMIT uploadProgress(sentBytes, totalBytes);
    } else if (result.msg == QStringLiteral("opp_complete")) {
        qBBBluetoothDebug() << "opp complete";
        m_finished = true;
        m_running = false;
        Q_EMIT finished(this);
    }
}

/*!
    Returns true if this reply has finished; otherwise returns false.
*/
bool QBluetoothTransferReplyQnx::isFinished() const
{
    return m_finished;
}

/*!
    Returns true if this reply is running; otherwise returns false.
*/
bool QBluetoothTransferReplyQnx::isRunning() const
{
    return m_running;
}

void QBluetoothTransferReplyQnx::abort()
{
    //not supported yet
}

#include "moc_qbluetoothtransferreply_qnx_p.cpp"

QT_END_NAMESPACE_BLUETOOTH
