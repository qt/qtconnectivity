/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Mobility Components.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qbluetooth.h"

/*!
    \namespace QBluetooth
    \brief The QBluetooth namespace contains functions and definitions related to Bluetooth.
    \since 5.0

    \ingroup connectivity-bluetooth
    \inmodule QtConnectivity
*/

/*!
    \enum QBluetooth::Security

    This enum describe the security requirements of a Bluetooth service.

    \value NoSecurity       The service does not require any security.

    \value Authorization The service requires authorization. Device does not
    have to paired, the connection will be granted by prompting the user unless
    the device is Authorized-Paired where the connection will be made
    automatically.

    \value Authentication The service requires authentication. Device must
    paired, the user maybe prompted on connection unless the device is
    Authorized-Paired.

    \value Encryption The service requires that the communications link be
    encrypted.  This requires the device be paired.

    \value Secure The service requires that the communications link be secure.
    Legacy pairing is not permitted, Simple Pairing from Bluetooth 2.1 or
    greater is required.
*/
