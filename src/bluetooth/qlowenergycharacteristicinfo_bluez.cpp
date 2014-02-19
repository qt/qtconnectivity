/***************************************************************************
**
** Copyright (C) 2013 BlackBerry Limited all rights reserved
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

#include "qlowenergycharacteristicinfo.h"
#include "qlowenergycharacteristicinfo_p.h"
#include "bluez/characteristic_p.h"
#include "qlowenergyprocess_p.h"

#include <QtDBus/QDBusPendingCallWatcher>

#define QT_LOWENERGYCHARACTERISTIC_DEBUG

#ifdef QT_LOWENERGYCHARACTERISTIC_DEBUG
#include <QtCore/QDebug>
#endif

QT_BEGIN_NAMESPACE

QLowEnergyCharacteristicInfoPrivate::QLowEnergyCharacteristicInfoPrivate()
    :   permission(0), notification (false), handle(QStringLiteral("0x0000")),
        properties(QVariantMap()), characteristic(0), m_signalConnected(false)
{
    process = process->instance();
    t=0;
}

QLowEnergyCharacteristicInfoPrivate::~QLowEnergyCharacteristicInfoPrivate()
{
    delete characteristic;
}

void QLowEnergyCharacteristicInfoPrivate::replyReceived(const QString &reply)
{
    QStringList update, row;
    update = QStringList();
    row = QStringList();
    if (reply.contains(QStringLiteral("[CON]"))) {
#ifdef QT_LOWENERGYCHARACTERISTIC_DEBUG
        qDebug() << "Char connected" << t;
#endif
    }
    if (reply.contains(QStringLiteral("[CON]")) && t == 1) {
        QString command = QStringLiteral("char-read-hnd ") + handle;
        process->executeCommand(command);
        process->executeCommand(QStringLiteral("\n"));
        t++;
    }
    if (reply.contains(QStringLiteral("Notification handle"))) {
        update = reply.split(QStringLiteral("\n"));
#ifdef QT_LOWENERGYCHARACTERISTIC_DEBUG
        qDebug() << update.size();
#endif
        for (int i = 0; i< update.size(); i ++) {
            if (update.at(i).contains(QStringLiteral("Notification handle"))) {
                row = update.at(i).split(QRegularExpression(QStringLiteral("\\W+")), QString::SkipEmptyParts);
#ifdef QT_LOWENERGYCHARACTERISTIC_DEBUG
                qDebug() << "Handle : "<< handle << row;
#endif
                if (row.at(2) == handle) {
                    QString notificationValue = QStringLiteral("");
                    for (int j = 4 ; j< row.size(); j++)
                        notificationValue += row.at(j);
#ifdef QT_LOWENERGYCHARACTERISTIC_DEBUG
                    qDebug() << notificationValue;
#endif
                    value = notificationValue.toUtf8();
                    properties[QStringLiteral("value")] = value;
                    emit notifyValue(uuid);
                }
            }

        }
    }

    if (reply.contains(QStringLiteral("Characteristic value/descriptor:"))) {
        update = reply.split(QStringLiteral("\n"));
        for (int i = 0; i< update.size(); i ++) {
            if (update.at(i).contains(QStringLiteral("Characteristic value/descriptor:"))) {
                row = update.at(i).split(QRegularExpression(QStringLiteral("\\W+")), QString::SkipEmptyParts);
                QString val = QStringLiteral("");
                for ( int j = 3; j<row.size(); j++)
                   val += row.at(j);
#ifdef QT_LOWENERGYCHARACTERISTIC_DEBUG
                   qDebug() << "HANDLE: " << uuid << val;
#endif
                   value = val.toUtf8();
                   properties[QStringLiteral("value")] = value;
            }
        }
    }
}

void QLowEnergyCharacteristicInfoPrivate::setValue(const QByteArray &wantedValue)
{
    if (permission & QLowEnergyCharacteristicInfo::Write) {
        process = process->instance();
        if (!m_signalConnected) {
            connect(process, SIGNAL(replySend(const QString &)), this, SLOT(replyReceived(const QString &)));
            m_signalConnected = true;
        }
        value = wantedValue;
        QString command;
        if (notification == true)
            command = QStringLiteral("char-write-req ") + notificationHandle + QStringLiteral(" ") + QString::fromLocal8Bit(value.constData());
        else
            command = QStringLiteral("char-write-req ") + handle + QStringLiteral(" ") + QString::fromLocal8Bit(value.constData());


    #ifdef QT_LOWENERGYCHARACTERISTIC_DEBUG
        qDebug() << command << t << process;
    #endif
        process->executeCommand(command);
        process->executeCommand(QStringLiteral("\n"));
        t++;
    }
    else {
        errorString = QStringLiteral("This characteristic does not support write operations.");
        emit error(uuid);
    }
}

bool QLowEnergyCharacteristicInfoPrivate::enableNotification()
{
    if (!notification)
        return false;
    /*
     *  Wanted value to enable notifications is 0100
     */
    if ( (permission & QLowEnergyCharacteristicInfo::Notify) == 0) {
#ifdef QT_LOWENERGYCHARACTERISTIC_DEBUG
        qDebug() << "Notification changes not allowed";
#endif
        errorString = QStringLiteral("This characteristic does not support notifications.");
        emit error(uuid);
        return false;
    }
    QByteArray val;
    val.append(48);
    val.append(49);
    val.append(48);
    val.append(48);
    setValue(val);
    return true;
}

void QLowEnergyCharacteristicInfoPrivate::disableNotification()
{
    /*
     *  Wanted value to disable notifications is 0000
     */
    QByteArray val;
    val.append(48);
    val.append(48);
    val.append(48);
    val.append(48);
    setValue(val);
}

void QLowEnergyCharacteristicInfoPrivate::readDescriptors()
{

}

void QLowEnergyCharacteristicInfoPrivate::readValue()
{
    QString command = QStringLiteral("char-read-hnd ") + handle;
    process->executeCommand(command);
    process->executeCommand(QStringLiteral("\n"));
}

bool QLowEnergyCharacteristicInfoPrivate::valid()
{
    return true;
}

QT_END_NAMESPACE
