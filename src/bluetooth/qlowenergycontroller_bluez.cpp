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

#include "qlowenergycontroller.h"
#include "qlowenergycontroller_p.h"
#include "qlowenergyserviceinfo_p.h"
#include "qlowenergycharacteristicinfo_p.h"
#include "qlowenergyprocess_p.h"
#include "qlowenergydescriptorinfo.h"
#include "qlowenergydescriptorinfo_p.h"
#include "moc_qlowenergycontroller.cpp"

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_BLUEZ)

QLowEnergyControllerPrivate::QLowEnergyControllerPrivate():
    m_randomAddress(false), m_step(0), m_commandStarted(false)
{

}

QLowEnergyControllerPrivate::~QLowEnergyControllerPrivate()
{

}

void QLowEnergyControllerPrivate::connectService(const QLowEnergyServiceInfo &service)
{
    errorString = QString();
    service.d_ptr->characteristicList.clear();
    bool add = true;
    for (int i = 0; i < m_leServices.size(); i++) {
        if (m_leServices.at(i).serviceUuid() == service.serviceUuid()) {
            m_leServices.replace(i, service);
            add = false;
            break;
        }
    }
    if (add)
        m_leServices.append(service);
    process = process->instance();
    if (!process->isConnected()) {
        if (m_commandStarted)
            connectToTerminal();
        else {
            QObject::connect(process, SIGNAL(replySend(const QString &)), q_ptr, SLOT(_q_replyReceived(const QString &)));
            QString command = QStringLiteral("gatttool -i ") + service.d_ptr->adapterAddress.toString() + QStringLiteral(" -b ") + service.d_ptr->deviceInfo.address().toString() + QStringLiteral(" -I");
            if (m_randomAddress)
                command += QStringLiteral(" -t random");
            qCDebug(QT_BT_BLUEZ) << "[REGISTER] uuid inside: " << service.d_ptr->uuid << command;
            process->startCommand(command);
            m_commandStarted = true;
        }
        process->addConnection();
    }
    else
        service.d_ptr->m_step = 1;
}

void QLowEnergyControllerPrivate::disconnectService(const QLowEnergyServiceInfo &service)
{
    if (service.isValid()) {
        for (int i = 0; i < m_leServices.size(); i++) {
            if (m_leServices.at(i).serviceUuid() == service.serviceUuid() && service.isConnected()) {
                process = process->instance();
                process->endProcess();
                m_leServices.at(i).d_ptr->connected = false;
                m_leServices.at(i).d_ptr->m_step = 0;
                m_leServices.at(i).d_ptr->m_valueCounter = 0;
                m_leServices.removeAt(i);
                emit q_ptr->disconnected(service);
                break;
            }
        }
    }
    else
        disconnectAllServices();
}

void QLowEnergyControllerPrivate::_q_replyReceived(const QString &reply)
{
    qCDebug(QT_BT_BLUEZ) << "reply: " << reply;
    // STEP 0
    if (m_step == 0 && !m_leServices.isEmpty())
        connectToTerminal();
    if (reply.contains(QStringLiteral("Connection refused (111)"))) {
        errorString = QStringLiteral("Connection refused (111)");
        disconnectAllServices();
    }
    else if (reply.contains(QStringLiteral("Device busy"))) {
        errorString = QStringLiteral("Connection refused (111)");
        disconnectAllServices();
    }
    else if (reply.contains(QStringLiteral("disconnected"))) {
        errorString = QStringLiteral("Trying to execute command on disconnected service");
        disconnectAllServices();
    }
    else {
        // STEP 1
        if (reply.contains(QStringLiteral("[CON]"))) {
            for (int i = 0; i < m_leServices.size(); i++) {
                if (m_leServices.at(i).d_ptr->m_step == 1) {
                    setHandles();
                    m_leServices.at(i).d_ptr->m_step++;
                }
            }
        }

        // STEP 2
        if (reply.contains(QStringLiteral("attr")) && reply.contains(QStringLiteral("uuid"))){
            const QStringList handles = reply.split(QStringLiteral("\n"));
            foreach (const QString &handle, handles) {
                if (handle.contains(QStringLiteral("attr")) && handle.contains(QStringLiteral("uuid"))) {
                    const QStringList handleDetails = handle.split(QRegularExpression(QStringLiteral("[\\s+|,]")),
                                                                   QString::SkipEmptyParts);
                    Q_ASSERT(handleDetails.count() == 9);
                    const QBluetoothUuid foundUuid(handleDetails.at(8));
                    for (int i = 0; i < m_leServices.size(); i++) {
                        if (foundUuid == m_leServices.at(i).serviceUuid() && m_leServices.at(i).d_ptr->m_step == 2) {
                            m_leServices.at(i).d_ptr->startingHandle = handleDetails.at(2);
                            m_leServices.at(i).d_ptr->endingHandle = handleDetails.at(6);
                            setCharacteristics(i);
                        }
                    }
                }
            }
        }

        // STEP 3
        if (reply.contains(QStringLiteral("char")) && reply.contains(QStringLiteral("uuid")) && reply.contains(QStringLiteral("properties"))) {
            const QStringList handles = reply.split(QStringLiteral("\n"));
            int stepSet = -1;
            foreach (const QString& handle, handles) {
                if (handle.contains(QStringLiteral("char")) && handle.contains(QStringLiteral("uuid"))) {
                    const QStringList handleDetails = handle.split(QRegularExpression(QStringLiteral("[\\s+|,]")),
                                                                   QString::SkipEmptyParts);
                    Q_ASSERT(handleDetails.count() == 11);
                    const QString charHandle = handleDetails.at(8);
                    ushort charHandleId = charHandle.toUShort(0, 0);
                    for (int i = 0; i < m_leServices.size(); i++) {
                        if ( charHandleId >= m_leServices.at(i).d_ptr->startingHandle.toUShort(0,0) &&
                             charHandleId <= m_leServices.at(i).d_ptr->endingHandle.toUShort(0,0))
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
                            if (!(charInfo.d_ptr->permission & QLowEnergyCharacteristicInfo::Read))
                                qCDebug(QT_BT_BLUEZ) << "GATT characteristic: Read not permitted: " << charInfo.d_ptr->uuid;
                            else
                                m_leServices.at(i).d_ptr->m_readCounter++;
                            m_leServices.at(i).d_ptr->characteristicList.append(charInfo);
                            stepSet = i;
                            break;
                        }
                    }
                }
            }
            if ( stepSet != -1)
                readCharacteristicValue(stepSet);
        }

        // STEP 4  and STEP 5
        // This part is for reading characteristic values and setting the notifications
        if (reply.contains(QStringLiteral("handle")) && reply.contains(QStringLiteral("value")) && !reply.contains(QStringLiteral("char"))) {
            int index = -1; // setting characteristics values
            int index1 = -1; // setting notifications and descriptors
            for (int i = 0; i < m_leServices.size(); i++) {
                if (m_leServices.at(i).d_ptr->m_step == 4)
                    index = i;
                if (m_leServices.at(i).d_ptr->m_step == 5)
                    index1 = i;
            }
            const QStringList handles = reply.split(QStringLiteral("\n"));
            bool lastStep = false;
            foreach (const QString &handle, handles) {
                if (handle.contains(QStringLiteral("handle")) &&
                        handle.contains(QStringLiteral("value")))
                {
                    const QStringList handleDetails = handle.split(QRegularExpression(QStringLiteral("\\W+")), QString::SkipEmptyParts);
                    if (index != -1) {
                        if (!m_leServices.at(index).d_ptr->characteristicList.isEmpty()) {
                            for (int j = 0; j < m_leServices.at(index).d_ptr->characteristicList.size(); j++) {
                                const QLowEnergyCharacteristicInfo chars = m_leServices.at(index).d_ptr->characteristicList.at(j);
                                const QString handleId = handleDetails.at(1);
                                qCDebug(QT_BT_BLUEZ) << handleId << handleId.toUShort(0,0) << chars.handle() << chars.handle().toUShort(0,0);
                                if (handleId.toUShort(0,0) == chars.handle().toUShort(0,0)) {
                                    QString value;
                                    for ( int k = 3; k < handleDetails.size(); k++)
                                        value = value + handleDetails.at(k);
                                    m_leServices.at(index).d_ptr->characteristicList[j].d_ptr->value = value.toUtf8();
                                    qCDebug(QT_BT_BLUEZ) << "Characteristic value set." << m_leServices.at(index).d_ptr->characteristicList[j].uuid() << chars.d_ptr->handle << value;
                                    m_leServices.at(index).d_ptr->m_valueCounter++;
                                }
                            }
                        }
                        if (m_leServices.at(index).d_ptr->m_valueCounter == m_leServices.at(index).d_ptr->m_readCounter) {
                            setNotifications();
                            m_leServices.at(index).d_ptr->m_step++;
                        }
                    }
                    if (index1 != -1) {
                        for (int j = 0; j < (m_leServices.at(index1).d_ptr->characteristicList.size()-1); j++) {
                            QLowEnergyCharacteristicInfo chars = m_leServices.at(index1).d_ptr->characteristicList.at(j);
                            QLowEnergyCharacteristicInfo charsNext = m_leServices.at(index1).d_ptr->characteristicList.at(j+1);
                            const QString handleId = handleDetails.at(1);
                            ushort h = handleId.toUShort(0, 0);
                            qCDebug(QT_BT_BLUEZ) << handleId << h  << chars.handle() << chars.handle().toUShort(0,0);

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
                                m_leServices.at(index1).d_ptr->characteristicList[j].d_ptr->descriptorsList.append(descriptor);
                                qCDebug(QT_BT_BLUEZ) << "Notification characteristic set." << chars.d_ptr->handle << chars.d_ptr->notificationHandle;
                            }
                        }
                        if (!lastStep) {
                            m_leServices.at(index1).d_ptr->m_step++;
                            m_leServices.at(index1).d_ptr->connected = true;
                            emit q_ptr->connected(m_leServices.at(index1));
                        }
                        lastStep = true;
                    }
                }
            }
        }

        // READING ADVERTISEMENT FROM THE BLE DEVICE
        if (reply.contains(QStringLiteral("Notification handle"))) {
            QStringList update, row;
            update = reply.split(QStringLiteral("\n"));
            for (int i = 0; i< update.size(); i ++) {
                if (update.at(i).contains(QStringLiteral("Notification handle"))) {
                    row = update.at(i).split(QRegularExpression(QStringLiteral("\\W+")), QString::SkipEmptyParts);
                    for (int j = 0; j < m_leServices.size(); j++) {
                        for (int k = 0; k < m_leServices.at(j).characteristics().size(); k++) {
                            if (m_leServices.at(j).characteristics().at(k).handle() == row.at(2)) {

                                QString notificationValue = QStringLiteral("");
                                for (int s = 4 ; s< row.size(); s++)
                                    notificationValue += row.at(s);
                                m_leServices.at(j).characteristics().at(k).d_ptr->value = notificationValue.toUtf8();
                                m_leServices.at(j).characteristics().at(k).d_ptr->properties[QStringLiteral("value")] = notificationValue.toUtf8();
                                emit q_ptr->valueChanged(m_leServices.at(j).characteristics().at(k));
                            }
                        }
                    }
                }

            }
        }
    }
}

void QLowEnergyControllerPrivate::disconnectAllServices()
{
    for (int i = 0; i < m_leServices.size(); i++) {
        process = process->instance();
        process->endProcess();
        m_leServices.at(i).d_ptr->m_step = 0;
        m_leServices.at(i).d_ptr->m_valueCounter = 0;
        if (m_leServices.at(i).isConnected()) {
            m_leServices.at(i).d_ptr->connected = false;
            emit q_ptr->disconnected(m_leServices.at(i));
        }
        if (errorString != QString())
            emit q_ptr->error(m_leServices.at(i));
        m_step = 0;
    }
    m_leServices.clear();
}

void QLowEnergyControllerPrivate::connectToTerminal()
{
    process->executeCommand(QStringLiteral("connect\n"));
    for (int i = 0; i < m_leServices.size(); i++)
        m_leServices.at(i).d_ptr->m_step = 1;
    m_step++;
}

void QLowEnergyControllerPrivate::setHandles()
{
    process->executeCommand(QStringLiteral("primary\n"));
    m_step++;
}

void QLowEnergyControllerPrivate::setCharacteristics(int a)
{
    const QString command = QStringLiteral("characteristics ") + m_leServices.at(a).d_ptr->startingHandle + QStringLiteral(" ") + m_leServices.at(a).d_ptr->endingHandle;
    process->executeCommand(command);
    process->executeCommand(QStringLiteral("\n"));
    m_leServices.at(a).d_ptr->m_step++;
}

void QLowEnergyControllerPrivate::setNotifications()
{
    process->executeCommand(QStringLiteral("char-read-uuid 2902\n"));
}

void QLowEnergyControllerPrivate::readCharacteristicValue(int index)
{
    for (int i = 0; i < m_leServices.at(index).d_ptr->characteristicList.size(); i++) {
        if ((m_leServices.at(index).d_ptr->characteristicList.at(i).d_ptr->permission & QLowEnergyCharacteristicInfo::Read)) {
            const QString uuidHandle = m_leServices.at(index).d_ptr->characteristicList.at(i).uuid().toString().remove(QLatin1Char('{')).remove(QLatin1Char('}'));
            const QString command = QStringLiteral("char-read-uuid ") + uuidHandle;
            process->executeCommand(command);
            process->executeCommand(QStringLiteral("\n"));
        }
    }
    m_leServices.at(index).d_ptr->m_step++;
}

void QLowEnergyControllerPrivate::writeValue(const QString &handle, const QByteArray &value)
{
    process = process->instance();
    QString command = QStringLiteral("char-write-req ") + handle + QStringLiteral(" ") + QString::fromLocal8Bit(value.constData());
    process->executeCommand(command);
    process->executeCommand(QStringLiteral("\n"));

}

bool QLowEnergyControllerPrivate::enableNotification(const QLowEnergyCharacteristicInfo &characteristic)
{
    const QBluetoothUuid descUuid((ushort)0x2902);
    for (int i = 0; i < m_leServices.size(); i++) {
        for (int j = 0; j < m_leServices.at(i).characteristics().size(); j++) {
            if (m_leServices.at(i).characteristics().at(j).uuid() == characteristic.uuid()) {
                for (int k = 0; k < m_leServices.at(i).characteristics().at(j).descriptors().size(); k++) {
                    if (m_leServices.at(i).characteristics().at(j).descriptors().at(k).uuid() == descUuid){
                        QByteArray val;
                        val.append(48);
                        val.append(49);
                        val.append(48);
                        val.append(48);
                        writeValue(m_leServices.at(i).characteristics().at(j).descriptors().at(k).handle(), val);
                        return true;
                    }
                }
            }
        }
    }

    errorString = QStringLiteral("Characteristic or notification descriptor not found.");
    emit q_ptr->error(characteristic);
    return false;
}

void QLowEnergyControllerPrivate::disableNotification(const QLowEnergyCharacteristicInfo &characteristic)
{
    const QBluetoothUuid descUuid((ushort)0x2902);
    for (int i = 0; i < m_leServices.size(); i++) {
        for (int j = 0; j < m_leServices.at(i).characteristics().size(); j++) {
            if (m_leServices.at(i).characteristics().at(j).uuid() == characteristic.uuid()) {
                for (int k = 0; k < m_leServices.at(i).characteristics().at(j).descriptors().size(); k++) {
                    if (m_leServices.at(i).characteristics().at(j).descriptors().at(k).uuid() == descUuid){
                        QByteArray val;
                        val.append(48);
                        val.append(48);
                        val.append(48);
                        val.append(48);
                        writeValue(m_leServices.at(i).characteristics().at(j).descriptors().at(k).handle(), val);
                    }
                }
            }
        }
    }
}

bool QLowEnergyControllerPrivate::write(const QLowEnergyCharacteristicInfo &characteristic)
{
    if (process->isConnected() && characteristic.isValid()) {
        if (QLowEnergyCharacteristicInfo::Write & characteristic.permissions()) {
            writeValue(characteristic.handle(), characteristic.value());
            return true;
        } else {
            errorString = QStringLiteral("This characteristic does not support write operations.");
        }
    } else {
        errorString = QStringLiteral("The device is not connected or characteristic is not valid");
    }

    emit q_ptr->error(characteristic);
    return false;
}

bool QLowEnergyControllerPrivate::write(const QLowEnergyDescriptorInfo &descriptor)
{
    if (process->isConnected()) {
        writeValue(descriptor.handle(), descriptor.value());
        return true;
    }

    return false;
}

QT_END_NAMESPACE
