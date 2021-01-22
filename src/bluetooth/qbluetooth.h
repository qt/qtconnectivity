/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
****************************************************************************/

#ifndef QBLUETOOTH_H
#define QBLUETOOTH_H

#include <QtBluetooth/qtbluetoothglobal.h>

QT_BEGIN_NAMESPACE

namespace QBluetooth {

// TODO Qt 6: Merge these two enums? But note that ATT Authorization has no equivalent
//            on the socket security level.

enum Security {
    NoSecurity = 0x00,
    Authorization = 0x01,
    Authentication = 0x02,
    Encryption = 0x04,
    Secure = 0x08
};

Q_DECLARE_FLAGS(SecurityFlags, Security)
Q_DECLARE_OPERATORS_FOR_FLAGS(SecurityFlags)

enum AttAccessConstraint {
    AttAuthorizationRequired = 0x1,
    AttAuthenticationRequired = 0x2,
    AttEncryptionRequired = 0x4,
};

Q_DECLARE_FLAGS(AttAccessConstraints, AttAccessConstraint)
Q_DECLARE_OPERATORS_FOR_FLAGS(AttAccessConstraints)

}

typedef quint16 QLowEnergyHandle;

QT_END_NAMESPACE

#endif // QBLUETOOTH_H
