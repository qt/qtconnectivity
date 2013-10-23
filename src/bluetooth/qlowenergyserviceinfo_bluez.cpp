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
        QString command;
        if (randomAddress)
            command = QStringLiteral("gatttool -i ") + adapterAddress.toString() + QStringLiteral(" -b ") + deviceInfo.address().toString() +QStringLiteral(" -I -t random");
        else
            command = QStringLiteral("gatttool -i ") + adapterAddress.toString() + QStringLiteral(" -b ") + deviceInfo.address().toString() + QStringLiteral(" -I");
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
        //process->executeCommand(QStringLiteral("disconnect"));
        //process->executeCommand(QStringLiteral("\n"));
        //process->executeCommand(QStringLiteral("exit"));
        //process->executeCommand(QStringLiteral("\n"));
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
            QStringList chars = reply.split(QStringLiteral("\n"));
            for (int i = 1; i<chars.size(); i++){
                if (chars.at(i).contains(QStringLiteral("attr")) && chars.at(i).contains(QStringLiteral("uuid"))){
                    QStringList l = chars.at(i).split(QRegularExpression(QStringLiteral("\\W+")), QString::SkipEmptyParts);
                    QString checkUuid = l.at(8) + QStringLiteral("-") + l.at(9) + QStringLiteral("-") + l.at(10) + QStringLiteral("-") + l.at(11) + QStringLiteral("-") + l.at(12);
                    if ( checkUuid == uuid.toString().remove(QLatin1Char('{')).remove(QLatin1Char('}'))) {
                        startingHandle = l.at(2);
                        endingHandle = l.at(6);
                        stepSet = true;
                    }
                }
            }
            if (stepSet)
                setCharacteristics();
        }
        break;
    case 3:
        stepSet = false;
        if (reply.contains(QStringLiteral("char")) && reply.contains(QStringLiteral("uuid"))) {
            QStringList chars = reply.split(QStringLiteral("\n"));
            for (int i = 1; i<chars.size(); i++){

                if (chars.at(i).contains(QStringLiteral("char")) && chars.at(i).contains(QStringLiteral("uuid"))){
                    QStringList l = chars.at(i).split(QRegularExpression(QStringLiteral("\\W+")), QString::SkipEmptyParts);
#ifdef QT_LOWENERGYSERVICE_DEBUG
                    qDebug() <<l;
#endif
                    QString charHandle = l.at(8);
                    if ( charHandle.toUShort(0,0) >= startingHandle.toUShort(0,0) && charHandle.toUShort(0,0) <= endingHandle.toUShort(0,0)) {
                        QString uuidParts = l.at(10) + "-" + l.at(11) + "-" + l.at(12) + "-" + l.at(13) + "-" + l.at(14);
                        QBluetoothUuid charUuid(uuidParts);
                        QVariantMap map = QVariantMap();

                        QLowEnergyCharacteristicInfo chars(charUuid);
                        chars.d_ptr->handle = charHandle;
                        chars.d_ptr->startingHandle = l.at(1);
                        QString perm = l.at(4);
                        chars.d_ptr->permission = perm.toUShort(0,0);
                        map[QStringLiteral("uuid")] = charUuid.toString();
                        map[QStringLiteral("handle")] = charHandle;
                        map[QStringLiteral("permission")] = chars.d_ptr->permission;
                        chars.d_ptr->properties = map;
                        if ((chars.d_ptr->permission & QLowEnergyCharacteristicInfo::Read) == 0) {
#ifdef QT_LOWENERGYSERVICE_DEBUG
                            qDebug() << "GATT characteristic: Read not permitted: " << chars.d_ptr->uuid;
#endif
                        }
                        else
                            m_readCounter++;
                        characteristicList.append(chars);
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
        stepSet = false;
        if (reply.contains(QStringLiteral("handle")) && reply.contains(QStringLiteral("value"))) {
            QStringList chars = reply.split(QStringLiteral("\n"));
            for (int i = 1; i<chars.size(); i++){

                if (chars.at(i).contains(QStringLiteral("handle")) && chars.at(i).contains(QStringLiteral("value"))){
                    QStringList l = chars.at(i).split(QRegularExpression(QStringLiteral("\\W+")), QString::SkipEmptyParts);
                    if (!characteristicList.isEmpty()) {
                        for (int j = 0; j < (characteristicList.size()-1); j++) {
                            QLowEnergyCharacteristicInfo chars((QLowEnergyCharacteristicInfo)characteristicList.at(j));
#ifdef QT_LOWENERGYSERVICE_DEBUG
                            qDebug() << l.at(1) << l.at(1).toUShort(0,0)<< chars.handle() << chars.handle().toUShort(0,0);
#endif
                            QLowEnergyCharacteristicInfo charsNext = (QLowEnergyCharacteristicInfo)characteristicList.at(j+1);
                            if (l.at(1).toUShort(0,0) > chars.handle().toUShort(0,0) && l.at(1).toUShort(0,0) < charsNext.handle().toUShort(0,0)) {
                                chars.d_ptr->notificationHandle = l.at(1);
                                chars.d_ptr->notification = true;
                                QString notUuid = QStringLiteral("0x2902");
                                QBluetoothUuid descUuid(notUuid.toUShort(0,0));
                                QLowEnergyDescriptorInfo descriptor(descUuid, l.at(1));
                                QString val = QStringLiteral("");
                                for (int k = 0; k < l.size(); k++)
                                    val = val + l.at(k);
                                descriptor.d_ptr->m_value = val.toUtf8();
                                QVariantMap map = QVariantMap();
                                map[QStringLiteral("uuid")] = descUuid.toString();
                                map[QStringLiteral("handle")] = l.at(1);
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
            QStringList chars = reply.split(QStringLiteral("\n"));
            for (int i = 1; i<chars.size(); i++){

                if (chars.at(i).contains(QStringLiteral("handle")) && chars.at(i).contains(QStringLiteral("value"))){
                    QStringList l = chars.at(i).split(QRegularExpression(QStringLiteral("\\W+")), QString::SkipEmptyParts);
                    if (!characteristicList.isEmpty()) {
                        for (int j = 0; j < characteristicList.size(); j++) {
                            QLowEnergyCharacteristicInfo chars((QLowEnergyCharacteristicInfo)characteristicList.at(j));
#ifdef QT_LOWENERGYSERVICE_DEBUG
                            qDebug() << l.at(1) << l.at(1).toUShort(0,0)<< chars.handle() << chars.handle().toUShort(0,0);
#endif
                            if (((QString)l.at(1)).toUShort(0,0) == chars.handle().toUShort(0,0)) {
                                QString value = QStringLiteral("");
                                for ( int k = 3; k < l.size(); k++)
                                    value = value + l.at(k);
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
    QString command = QStringLiteral("primary ");
#ifdef QT_LOWENERGYSERVICE_DEBUG
    qDebug() << "Setting handles: " << command;
#endif
    process->executeCommand(command);
    process->executeCommand(QStringLiteral("\n"));
    m_step++;
}

void QLowEnergyServiceInfoPrivate::setCharacteristics()
{
    QString command = QStringLiteral("characteristics ") + startingHandle + QStringLiteral(" ") + endingHandle;
#ifdef QT_LOWENERGYSERVICE_DEBUG
    qDebug() << "Setting characteristics: " <<command;
#endif
    process->executeCommand(command);
    process->executeCommand(QStringLiteral("\n"));
    m_step++;
}

void QLowEnergyServiceInfoPrivate::setNotifications()
{
    QString command = QStringLiteral("char-read-uuid 2902");
    process->executeCommand(command);
    process->executeCommand(QStringLiteral("\n"));
    m_step++;
}

void QLowEnergyServiceInfoPrivate::readCharacteristicValue()
{
    for (int i = 0; i < characteristicList.size(); i++) {
        if ((characteristicList.at(i).d_ptr->permission & QLowEnergyCharacteristicInfo::Read) != 0) {
            QString uuidHandle = characteristicList.at(i).uuid().toString().remove(QLatin1Char('{')).remove(QLatin1Char('}'));
            QString command = QStringLiteral("char-read-uuid ") + uuidHandle;
            process->executeCommand(command);
            process->executeCommand(QStringLiteral("\n"));
        }
    }
    m_step++;
}

void QLowEnergyServiceInfoPrivate::readDescriptors()
{
    QString command = QStringLiteral("char-desc ") + startingHandle + QStringLiteral(" ") + endingHandle;
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
