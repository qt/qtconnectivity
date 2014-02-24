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

#include "qlowenergyserviceinfo_p.h"
#include "qlowenergyserviceinfo.h"
#include "qlowenergycharacteristicinfo.h"
#include "qlowenergycharacteristicinfo_p.h"
#include "qlowenergyprocess_p.h"
#include "qbluetoothlocaldevice.h"
#include "bluez/characteristic_p.h"
#include "qlowenergydescriptorinfo.h"
#include "qlowenergydescriptorinfo_p.h"

//#define QT_LOWENERGYSERVICE_DEBUG

#ifdef QT_LOWENERGYSERVICE_DEBUG
#include <QtCore/QDebug>
#endif

QT_BEGIN_NAMESPACE

QLowEnergyServiceInfoPrivate::QLowEnergyServiceInfoPrivate():
    serviceType(QLowEnergyServiceInfo::PrimaryService), connected(false), characteristic(0), m_valueCounter(0), m_readCounter(0)
{
    m_step = 0;
    randomAddress = false;
    QBluetoothLocalDevice localDevice;
    adapterAddress = localDevice.address();
}

QLowEnergyServiceInfoPrivate::QLowEnergyServiceInfoPrivate(const QString &servicePath):
    serviceType(QLowEnergyServiceInfo::PrimaryService), connected(false), path(servicePath), characteristic(0)
{
    m_step = 0;
    randomAddress = false;
}

QLowEnergyServiceInfoPrivate::~QLowEnergyServiceInfoPrivate()
{
    delete characteristic;
}

void QLowEnergyServiceInfoPrivate::registerServiceWatcher()
{
    characteristicList.clear();
    process = process->instance();
    if (!process->isConnected()) {
        connect(process, SIGNAL(replySend(const QString &)), this, SLOT(replyReceived(const QString &)));
        QString command = QStringLiteral("gatttool -i ") + adapterAddress.toString() + QStringLiteral(" -b ") + deviceInfo.address().toString() + QStringLiteral(" -I");
        if (randomAddress)
            command += QStringLiteral(" -t random");

#ifdef QT_LOWENERGYSERVICE_DEBUG
        qDebug() << "[REGISTER] uuid inside: " << uuid << command;
#endif
        process->startCommand(command);
        process->addConnection();
    }
    else {
        m_step++;
        connect(process, SIGNAL(replySend(const QString &)), this, SLOT(replyReceived(const QString &)));
    }
}

void QLowEnergyServiceInfoPrivate::unregisterServiceWatcher()
{
    if (connected) {
        process = process->instance();
        process->endProcess();
        connected = false;
        m_step = 0;
        m_valueCounter = 0;
        disconnect(process, SIGNAL(replySend(const QString &)), this, SLOT(replyReceived(const QString &)));
        emit disconnectedFromService(uuid);
    }
}

void QLowEnergyServiceInfoPrivate::replyReceived(const QString &reply)
{
#ifdef QT_LOWENERGYSERVICE_DEBUG
    qDebug() << uuid << "m step: " << m_step << reply;
#endif
    bool stepSet = false;
    switch (m_step) {
    case 0:
        connectToTerminal();
        break;
    case 1:
        if (reply.contains(QStringLiteral("[CON]"))) {
            connected = true;
            setHandles();
        }
        else {
            connected = false;
            if (reply.contains(QStringLiteral("Connection refused (111)"))) {
#ifdef QT_LOWENERGYSERVICE_DEBUG
                qDebug() << "Connection refused (111)";
#endif
                errorString = QStringLiteral("Connection refused (111)");
                emit error(uuid);
            }
            else if (reply.contains(QStringLiteral("Device busy"))) {
#ifdef QT_LOWENERGYSERVICE_DEBUG
                qDebug() << "Device busy";
#endif
                errorString = QStringLiteral("Connection refused (111)");
                emit error(uuid);
            }
        }
        break;
    case 2:
        if (reply.contains(QStringLiteral("attr")) && reply.contains(QStringLiteral("uuid"))){
            const QStringList handles = reply.split(QStringLiteral("\n"));
            foreach (const QString &handle, handles) {
                if (handle.contains(QStringLiteral("attr")) && handle.contains(QStringLiteral("uuid"))) {
                    const QStringList handleDetails = handle.split(QRegularExpression(QStringLiteral("[\\s+|,]")),
                                                                   QString::SkipEmptyParts);
                    Q_ASSERT(handleDetails.count() == 9);
                    const QBluetoothUuid foundUuid(handleDetails.at(8));
                    if (foundUuid == uuid) {
                        startingHandle = handleDetails.at(2);
                        endingHandle = handleDetails.at(6);
                        stepSet = true;
                    }
                }
            }
            if (stepSet)
                setCharacteristics();
        }
        break;
    case 3:
        if (reply.contains(QStringLiteral("char")) && reply.contains(QStringLiteral("uuid"))) {
            const QStringList handles = reply.split(QStringLiteral("\n"));
            foreach (const QString& handle, handles) {
                if (handle.contains(QStringLiteral("char")) && handle.contains(QStringLiteral("uuid"))) {
                    const QStringList handleDetails = handle.split(QRegularExpression(QStringLiteral("[\\s+|,]")),
                                                                   QString::SkipEmptyParts);
#ifdef QT_LOWENERGYSERVICE_DEBUG
                    qDebug() << handleDetails;
#endif
                    Q_ASSERT(handleDetails.count() == 11);
                    const QString charHandle = handleDetails.at(8);
                    ushort charHandleId = charHandle.toUShort(0, 0);
                    if ( charHandleId >= startingHandle.toUShort(0,0) &&
                            charHandleId <= endingHandle.toUShort(0,0))
                    {
                        const QBluetoothUuid charUuid(handleDetails.at(10));
                        QVariantMap map;

                        QLowEnergyCharacteristicInfo charInfo(charUuid);
                        charInfo.d_ptr->handle = charHandle;
                        charInfo.d_ptr->startingHandle = handleDetails.at(1);
                        charInfo.d_ptr->permission = handleDetails.at(4).toUShort(0,0);
                        map[QStringLiteral("uuid")] = charUuid.toString();
                        map[QStringLiteral("handle")] = charHandle;
                        map[QStringLiteral("permission")] = charInfo.d_ptr->permission;
                        charInfo.d_ptr->properties = map;
                        if (!(charInfo.d_ptr->permission & QLowEnergyCharacteristicInfo::Read)) {
#ifdef QT_LOWENERGYSERVICE_DEBUG
                            qDebug() << "GATT characteristic: Read not permitted: " << charInfo.d_ptr->uuid;
#endif
                        } else {
                            m_readCounter++;
                        }
                        characteristicList.append(charInfo);
                        stepSet = true;
                    }
                }
            }
#ifdef QT_LOWENERGYSERVICE_DEBUG
            qDebug() << "COUNTER : " << m_readCounter;
#endif
            if ( stepSet )
                setNotifications();
        }
        break;
    case 4:
        if (reply.contains(QStringLiteral("handle")) && reply.contains(QStringLiteral("value"))) {
            const QStringList handles = reply.split(QStringLiteral("\n"));
            foreach (const QString handle, handles) {
                if (handle.contains(QStringLiteral("handle")) && handle.contains(QStringLiteral("value"))) {
                    const QStringList handleDetails = handle.split(QRegularExpression(QStringLiteral("\\W+")), QString::SkipEmptyParts);
                    if (!characteristicList.isEmpty()) {
                        for (int j = 0; j < (characteristicList.size()-1); j++) {
                            QLowEnergyCharacteristicInfo chars = characteristicList.at(j);
                            QLowEnergyCharacteristicInfo charsNext = characteristicList.at(j+1);
                            const QString handleId = handleDetails.at(1);
                            ushort h = handleId.toUShort(0, 0);
#ifdef QT_LOWENERGYSERVICE_DEBUG
                            qDebug() << handleId << h  << chars.handle() << chars.handle().toUShort(0,0);
#endif
                            if (h > chars.handle().toUShort(0,0) && h < charsNext.handle().toUShort(0,0)) {
                                chars.d_ptr->notificationHandle = handleId;
                                chars.d_ptr->notification = true;
                                QBluetoothUuid descUuid((ushort)0x2902);
                                QLowEnergyDescriptorInfo descriptor(descUuid, handleId);
                                QString val;
                                //TODO why do we start parsing value from k = 0? Shouldn't it be k = 2
                                for (int k = 3; k < handleDetails.size(); k++)
                                    val = val + handleDetails.at(k);
                                descriptor.d_ptr->m_value = val.toUtf8();
                                QVariantMap map;
                                map[QStringLiteral("uuid")] = descUuid.toString();
                                map[QStringLiteral("handle")] = handleId;
                                map[QStringLiteral("value")] = val.toUtf8();
                                characteristicList[j].d_ptr->descriptorsList.append(descriptor);
#ifdef QT_LOWENERGYSERVICE_DEBUG
                                qDebug() << "Notification characteristic set." << chars.d_ptr->handle << chars.d_ptr->notificationHandle;
#endif
                            }
                        }
                    }
                }
            }
#ifdef QT_LOWENERGYSERVICE_DEBUG
            qDebug() << "COUNTER : " << m_readCounter;
#endif
            if (m_readCounter > 0)
                readCharacteristicValue();
            else {
#ifdef QT_LOWENERGYSERVICE_DEBUG
                qDebug() << "Ready to emit connected signal" << uuid;
#endif
                m_step = 6;
                connected = true;
                emit connectedToService(uuid);
            }
        }
        break;
    case 5:
        // This part is for reading characteristic values
        if (reply.contains(QStringLiteral("handle")) && reply.contains(QStringLiteral("value"))) {
            const QStringList handles = reply.split(QStringLiteral("\n"));
            foreach (const QString &handle, handles) {
                if (handle.contains(QStringLiteral("handle")) &&
                        handle.contains(QStringLiteral("value")))
                {
                    const QStringList handleDetails = handle.split(QRegularExpression(QStringLiteral("\\W+")), QString::SkipEmptyParts);
                    if (!characteristicList.isEmpty()) {
                        for (int j = 0; j < characteristicList.size(); j++) {
                            const QLowEnergyCharacteristicInfo chars = characteristicList.at(j);
                            const QString handleId = handleDetails.at(1);
#ifdef QT_LOWENERGYSERVICE_DEBUG
                            qDebug() << handleId << handleId.toUShort(0,0) << chars.handle() << chars.handle().toUShort(0,0);
#endif
                            if (handleId.toUShort(0,0) == chars.handle().toUShort(0,0)) {
                                QString value;
                                for ( int k = 3; k < handleDetails.size(); k++)
                                    value = value + handleDetails.at(k);
                                characteristicList[j].d_ptr->value = value.toUtf8();
#ifdef QT_LOWENERGYSERVICE_DEBUG
                                qDebug() << "Characteristic value set." << chars.d_ptr->handle << value;
#endif
                                m_valueCounter++;
                            }
                        }
                    }
                }
            }
            if (m_valueCounter == m_readCounter) {
                m_step++;
#ifdef QT_LOWENERGYSERVICE_DEBUG
                qDebug() << "Ready to emit connected signal" << uuid;
#endif
                connected = true;
                emit connectedToService(uuid);
            }
        }
        break;
    default:
        break;

    }

}

void QLowEnergyServiceInfoPrivate::connectToTerminal()
{
#ifdef QT_LOWENERGYSERVICE_DEBUG
    qDebug() << "[CONNECT TO TERMINAL] uuid inside: " << uuid;
#endif
    process->executeCommand(QStringLiteral("connect"));
    process->executeCommand(QStringLiteral("\n"));
    m_step++;
}

void QLowEnergyServiceInfoPrivate::setHandles()
{
#ifdef QT_LOWENERGYSERVICE_DEBUG
    qDebug() << "Setting handles: primary";
#endif
    process->executeCommand(QStringLiteral("primary"));
    process->executeCommand(QStringLiteral("\n"));
    m_step++;
}

void QLowEnergyServiceInfoPrivate::setCharacteristics()
{
    const QString command = QStringLiteral("characteristics ") + startingHandle + QStringLiteral(" ") + endingHandle;
#ifdef QT_LOWENERGYSERVICE_DEBUG
    qDebug() << "Setting characteristics: " << command;
#endif
    process->executeCommand(command);
    process->executeCommand(QStringLiteral("\n"));
    m_step++;
}

void QLowEnergyServiceInfoPrivate::setNotifications()
{
#ifdef QT_LOWENERGYSERVICE_DEBUG
    qDebug() << "Setting notifications: char-read-uuid 2902";
#endif
    process->executeCommand(QStringLiteral("char-read-uuid 2902"));
    process->executeCommand(QStringLiteral("\n"));
    m_step++;
}

void QLowEnergyServiceInfoPrivate::readCharacteristicValue()
{
    for (int i = 0; i < characteristicList.size(); i++) {
        if ((characteristicList.at(i).d_ptr->permission & QLowEnergyCharacteristicInfo::Read)) {
            const QString uuidHandle = characteristicList.at(i).uuid().toString().remove(QLatin1Char('{')).remove(QLatin1Char('}'));
#ifdef QT_LOWENERGYSERVICE_DEBUG
    qDebug() << "FFFFFF Setting notifications: char-read-uuid " << uuidHandle ;
#endif
            const QString command = QStringLiteral("char-read-uuid ") + uuidHandle;
            process->executeCommand(command);
            process->executeCommand(QStringLiteral("\n"));
        }
    }
    m_step++;
}

void QLowEnergyServiceInfoPrivate::readDescriptors()
{
    const QString command = QStringLiteral("char-desc ") + startingHandle + QStringLiteral(" ") + endingHandle;
    process->executeCommand(command);
    process->executeCommand(QStringLiteral("\n"));

    m_step++;
}

bool QLowEnergyServiceInfoPrivate::valid()
{
    if (adapterAddress == QBluetoothAddress())
        return false;
    return true;
}

QT_END_NAMESPACE
