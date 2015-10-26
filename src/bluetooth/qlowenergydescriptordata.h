/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
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
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QLOWENERGYDESCRIPTORDATA_H
#define QLOWENERGYDESCRIPTORDATA_H

#include <QtBluetooth/qbluetooth.h>
#include <QtBluetooth/qbluetoothuuid.h>
#include <QtCore/qshareddata.h>

QT_BEGIN_NAMESPACE

class QByteArray;
struct QLowEnergyDescriptorDataPrivate;

class Q_BLUETOOTH_EXPORT QLowEnergyDescriptorData
{
    friend Q_BLUETOOTH_EXPORT bool operator==(const QLowEnergyDescriptorData &d1,
                                              const QLowEnergyDescriptorData &d12);
public:
    QLowEnergyDescriptorData();
    QLowEnergyDescriptorData(const QBluetoothUuid &uuid,
                             const QByteArray &value);
    QLowEnergyDescriptorData(const QLowEnergyDescriptorData &other);
    ~QLowEnergyDescriptorData();

    QLowEnergyDescriptorData &operator=(const QLowEnergyDescriptorData &other);

    QByteArray value() const;
    void setValue(const QByteArray &value);

    QBluetoothUuid uuid() const;
    void setUuid(const QBluetoothUuid &uuid);

    bool isValid() const;

    // TODO: read permissions, write permissions, authentication/authorization (only applicable for some descriptors)

    void swap(QLowEnergyDescriptorData &other) Q_DECL_NOTHROW { qSwap(d, other.d); }

private:
    QSharedDataPointer<QLowEnergyDescriptorDataPrivate> d;
};

Q_BLUETOOTH_EXPORT bool operator==(const QLowEnergyDescriptorData &d1,
                                   const QLowEnergyDescriptorData &d2);

inline bool operator!=(const QLowEnergyDescriptorData &d1, const QLowEnergyDescriptorData &d2)
{
    return !(d1 == d2);
}

Q_DECLARE_SHARED(QLowEnergyDescriptorData)

QT_END_NAMESPACE

#endif // Include guard.
