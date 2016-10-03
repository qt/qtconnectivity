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

#ifndef OSXBLUETOOTH_P_H
#define OSXBLUETOOTH_P_H


//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qglobal.h>

#ifndef QT_OSX_BLUETOOTH

#include <CoreBluetooth/CoreBluetooth.h>

#else

#if QT_OSX_PLATFORM_SDK_EQUAL_OR_ABOVE(__MAC_10_12)

#include <CoreBluetooth/CoreBluetooth.h>
#include <IOBluetooth/IOBluetooth.h>

#else

// CoreBluetooth with SDK 10.9 seems to be broken: the class CBPeripheralManager is enabled on macOS
// but some of its declarations are using a disabled enum CBPeripheralAuthorizationStatus
// (disabled using __attribute__ syntax and NS_ENUM_AVAILABLE macro).
// This + -std=c++11 ends with a compilation error. For the SDK 10.9 we can:
// 1. either undefine NS_ENUM_AVAILABLE macro (it works somehow) and redefine it as an empty sequence
// of pp-tokens or
// 2. define __attribute__ as an empty sequence. Both solutions look quite ugly.

#if QT_OSX_PLATFORM_SDK_EQUAL_OR_ABOVE(__MAC_10_9) && !QT_OSX_PLATFORM_SDK_EQUAL_OR_ABOVE(__MAC_10_10)

// Must be included BEFORE CoreBluetooth.h:
#include <Foundation/Foundation.h>

#define CB_ERROR_WORKAROUND_REQUIRED
#undef NS_ENUM_AVAILABLE
#define NS_ENUM_AVAILABLE(_mac, _ios)

#endif // SDK version == 10.9

// In SDK below 10.12 IOBluetooth.h includes CoreBluetooth.h.
#include <IOBluetooth/IOBluetooth.h>

#ifdef CB_ERROR_WORKAROUND_REQUIRED
#undef __attribute__
#undef CB_ERROR_WORKAROUND_REQUIRED
#endif // WORKAROUND

#endif // SDK

#endif // QT_OSX_BLUETOOTH
#endif // OSXBLUETOOTH_P_H
