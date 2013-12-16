/***************************************************************************
**
** Copyright (C) 2013 BlackBerry Limited. All rights reserved.
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "characteristicinfo.h"
#include "qbluetoothuuid.h"
#include <QByteArray>

CharacteristicInfo::CharacteristicInfo():
    m_characteristic(QLowEnergyCharacteristicInfo())
{
}

CharacteristicInfo::CharacteristicInfo(const QLowEnergyCharacteristicInfo &characteristic):
    m_characteristic(characteristic)
{
    emit characteristicChanged();
}

void CharacteristicInfo::setCharacteristic(const QLowEnergyCharacteristicInfo &characteristic)
{
    m_characteristic = QLowEnergyCharacteristicInfo(characteristic);
    emit characteristicChanged();
}

QString CharacteristicInfo::getName() const
{
    return m_characteristic.name();
}

QString CharacteristicInfo::getUuid() const
{
    return m_characteristic.uuid().toString().remove(QLatin1Char('{')).remove(QLatin1Char('}'));
}

QString CharacteristicInfo::getValue() const
{
    // All characteristics values are in hexadecimal format.
    QString hexvalue = "";
    QByteArray a = m_characteristic.value();
    for (int i = 0; i < a.size(); i++){
        hexvalue.append(a.at(i));}
    return hexvalue;
}

QString CharacteristicInfo::getHandle() const
{
    return m_characteristic.handle();
}

QString CharacteristicInfo::getPermission() const
{
    QString properties = "Properties:";
    int permission = m_characteristic.permissions();
    if (permission & QLowEnergyCharacteristicInfo::Read)
        properties = properties + QStringLiteral(" Read");
    if (permission & QLowEnergyCharacteristicInfo::Write)
        properties = properties + QStringLiteral(" Write");
    if (permission & QLowEnergyCharacteristicInfo::Notify)
        properties = properties + QStringLiteral(" Notify");
    if (permission & QLowEnergyCharacteristicInfo::Indicate)
        properties = properties + QStringLiteral(" Indicate");
    if (permission & QLowEnergyCharacteristicInfo::ExtendedProperty)
        properties = properties + QStringLiteral(" ExtendedProperty");
    return properties;
}

QLowEnergyCharacteristicInfo CharacteristicInfo::getCharacteristic() const
{
    return m_characteristic;
}
