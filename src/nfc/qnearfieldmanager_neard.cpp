/***************************************************************************
**
** Copyright (C) 2014 BlackBerry Limited. All rights reserved.
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

#include "qnearfieldmanager_neard_p.h"
#include "qnearfieldtarget_neard_p.h"

#include "neard/manager_p.h"
#include "neard/adapter_p.h"

QT_BEGIN_NAMESPACE

// TODO We need a constructor that lets us select an adapter
QNearFieldManagerPrivateImpl::QNearFieldManagerPrivateImpl()
    : QNearFieldManagerPrivate(), m_adapter(0), m_manager(0)
{
    m_manager = new OrgNeardManagerInterface(QStringLiteral("org.neard"), QStringLiteral("/"),
                                                QDBusConnection::systemBus(), this);
    if (!m_manager->isValid()) {
        qDebug() << "Could not connect to manager" << "Neard daemon running?";
        return;
    }

    QDBusPendingReply<QVariantMap> reply = m_manager->GetProperties();
    reply.waitForFinished();
    if (reply.isError()) {
        qDebug() << "Error getting manager properties.";
        return;
    }

    // Now we check if the adapter still exists (it might have been unplugged)
    const QDBusArgument &paths = reply.value().value(QStringLiteral("Adapters")).value<QDBusArgument>();

    paths.beginArray();
    while (!paths.atEnd()) {
        QDBusObjectPath path;
        paths >> path;
        // Select the first adapter
        m_adapterPath = path.path();
        break;
    }
    paths.endArray();
}

QNearFieldManagerPrivateImpl::~QNearFieldManagerPrivateImpl()
{
    stopTargetDetection();
}

bool QNearFieldManagerPrivateImpl::isAvailable() const
{
    if (!m_manager->isValid() || m_adapterPath.isNull()) {
        return false;
    }

    QDBusPendingReply<QVariantMap> reply = m_manager->GetProperties();
    reply.waitForFinished();
    if (reply.isError()) {
        qDebug() << "Error getting manager properties.";
        return false;
    }

    // Now we check if the adapter still exists (it might have been unplugged)
    const QDBusArgument &paths = reply.value().value(QStringLiteral("Adapters")).value<QDBusArgument>();

    paths.beginArray();
    while (!paths.atEnd()) {
        QDBusObjectPath path;
        paths >> path;
        // Check if the adapter exists
        if (path.path() == m_adapterPath) {
            paths.endArray();
            return true;
        }
    }
    paths.endArray();
    return false;
}

bool QNearFieldManagerPrivateImpl::startTargetDetection()
{
    if (!isAvailable())
        return false;

    // TODO define behavior when target detection is started again
    if (m_adapter)
        return false;

    //qDebug() << "Constructing interface with path" << m_adapterPath;
    m_adapter = new OrgNeardAdapterInterface(QStringLiteral("org.neard"), m_adapterPath,
                                     QDBusConnection::systemBus(), this);

    {
        QDBusPendingReply<QVariantMap> reply = m_adapter->GetProperties();
        reply.waitForFinished();
        if (!reply.isError()) {
            if (reply.value().value(QStringLiteral("Polling")) == true) {
                //qDebug() << "Adapter" << m_adapterPath << "is busy";
                return false; // Should we return false or true?
            }
        }
    }

    // Switch on the NFC adapter if it is not already
    QDBusPendingReply<> reply = m_adapter->SetProperty(QStringLiteral("Powered"), QDBusVariant(true));
    reply.waitForFinished();

    //qDebug() << (reply.isError() ? "Error setting up adapter" : "Adapter powered on");
    reply = m_adapter->StartPollLoop(QStringLiteral("Dual"));
    reply.waitForFinished();
    //qDebug() << (reply.isError() ? "Error when starting polling" : "Successfully started to poll");

    // TagFound and TagLost DBus signals don't work
//    connect(m_adapter, SIGNAL(TagFound(const QDBusObjectPath&)),
//                            this, SLOT(tagFound(const QDBusObjectPath&)));
//    connect(m_adapter, SIGNAL(TagLost(const QDBusObjectPath&)),
//                            this, SLOT(tagFound(const QDBusObjectPath&)));
    connect(m_adapter, SIGNAL(PropertyChanged(QString,QDBusVariant)),
                            this, SLOT(propertyChanged(QString,QDBusVariant)));
    return true;
}

void QNearFieldManagerPrivateImpl::stopTargetDetection()
{
    if (!isAvailable())
        return;

    QDBusPendingReply<> reply = m_adapter->StopPollLoop();
    reply.waitForFinished();
    // TODO Should we really power down the adapter?
    reply = m_adapter->SetProperty(QStringLiteral("Powered"), QDBusVariant(false));
    reply.waitForFinished();
    delete m_adapter;
    m_adapter = 0;
}

int QNearFieldManagerPrivateImpl::registerNdefMessageHandler(QObject *object, const QMetaMethod &method)
{
    Q_UNUSED(object);
    Q_UNUSED(method);
    return 0;
}

int QNearFieldManagerPrivateImpl::registerNdefMessageHandler(const QNdefFilter &filter, QObject *object, const QMetaMethod &method)
{
    Q_UNUSED(filter);
    Q_UNUSED(object);
    Q_UNUSED(method);
    return -1;
}

bool QNearFieldManagerPrivateImpl::unregisterNdefMessageHandler(int handlerId)
{
    Q_UNUSED(handlerId);
    return false;
}

void QNearFieldManagerPrivateImpl::requestAccess(QNearFieldManager::TargetAccessModes accessModes)
{
    Q_UNUSED(accessModes);
}

void QNearFieldManagerPrivateImpl::releaseAccess(QNearFieldManager::TargetAccessModes accessModes)
{
    Q_UNUSED(accessModes);
}

void QNearFieldManagerPrivateImpl::tagFound(const QDBusObjectPath &path)
{
    qDebug() << "Tag found" << path.path();
}

void QNearFieldManagerPrivateImpl::propertyChanged(QString property, QDBusVariant value)
{
    qDebug() << "Property changed" << property;
    // New device detected
    if (property == QStringLiteral("Devices")) {
        const QDBusArgument &devices = value.variant().value<QDBusArgument>();
        QStringList devicesList;
        devices.beginArray();
        while (!devices.atEnd()) {
            QDBusObjectPath path;
            devices >> path;
            devicesList.append(path.path());
            qDebug() << "New device" << devicesList;
        }
        devices.endArray();
        qDebug() << "Devices list changed" << devicesList;
    } else if (property == QStringLiteral("Tags")) {
        const QDBusArgument &tags = value.variant().value<QDBusArgument>();
        QStringList tagList;
        tags.beginArray();
        while (!tags.atEnd()) {
            QDBusObjectPath path;
            tags >> path;
            tagList.append(path.path());
            qDebug() << "New tag" << path.path();
            NearFieldTarget<QNearFieldTarget> *nfTag =
                    new NearFieldTarget<QNearFieldTarget>(this, path);
            emit targetDetected(nfTag);
        }
        tags.endArray();
        qDebug() << "Tag list changed" << tagList;
    }
}

QT_END_NAMESPACE
