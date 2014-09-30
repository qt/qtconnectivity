/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
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

#ifndef OSXBTUTILITY_P_H
#define OSXBTUTILITY_P_H

#include <QtCore/qloggingcategory.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qglobal.h>

#include <Foundation/Foundation.h>
#include <IOBluetooth/Bluetooth.h>
#include <IOKit/IOReturn.h>

@class IOBluetoothSDPUUID;

QT_BEGIN_NAMESPACE

class QBluetoothUuid;

namespace OSXBluetooth {

struct NSObjectDeleter {
    static void cleanup(NSObject *obj)
    {
        [obj release];
    }
};

template<class T>
class ObjCScopedPointer : public QScopedPointer<NSObject, NSObjectDeleter>
{
public:
    // TODO: remove default argument, add 'retain' parameter,
    // add a default ctor??? This will make the semantics more
    // transparent + will simplify the future transition to ARC
    // (if it will ever happen).
    explicit ObjCScopedPointer(T *ptr = Q_NULLPTR) : QScopedPointer(ptr){}
    operator T*() const
    {
        return data();
    }

    T *data()const
    {
        return static_cast<T *>(QScopedPointer::data());
    }

    T *take()
    {
        return static_cast<T *>(QScopedPointer::take());
    }
};

typedef ObjCScopedPointer<NSAutoreleasePool> AutoreleasePool;
#define QT_BT_MAC_AUTORELEASEPOOL const OSXBluetooth::AutoreleasePool pool([[NSAutoreleasePool alloc] init])

template<class T>
class ObjCStrongReference {
public:
    ObjCStrongReference()
        : m_ptr(nil)
    {
    }
    ObjCStrongReference(T *obj, bool retain)
    {
        if (retain)
            m_ptr = [obj retain];
        else
            m_ptr = obj; // For example, created with initWithXXXX.
    }
    ObjCStrongReference(const ObjCStrongReference &rhs)
    {
        m_ptr = [rhs.m_ptr retain];
    }
    ObjCStrongReference &operator = (const ObjCStrongReference &rhs)
    {
        // "Old-style" implementation:
        if (this != &rhs && m_ptr != rhs.m_ptr) {
            [m_ptr release];
            m_ptr = [rhs.m_ptr retain];
        }

        return *this;
    }

#ifdef Q_COMPILER_RVALUE_REFS
    ObjCStrongReference(ObjCStrongReference &&xval)
    {
        m_ptr = xval.m_ptr;
        xval.m_ptr = nil;
    }

    ObjCStrongReference &operator = (ObjCStrongReference &&xval)
    {
        m_ptr = xval.m_ptr;
        xval.m_ptr = nil;
        return *this;
    }
#endif

    ~ObjCStrongReference()
    {
        [m_ptr release];
    }

    void reset(T *newVal)
    {
        if (m_ptr != newVal) {
            [m_ptr release];
            m_ptr = [newVal retain];
        }
    }

    operator T *() const
    {
        return m_ptr;
    }

    T *data() const
    {
        return m_ptr;
    }
private:
    T *m_ptr;
};

QString qt_address(NSString *address);
class QBluetoothAddress qt_address(const BluetoothDeviceAddress *address);
BluetoothDeviceAddress iobluetooth_address(const QBluetoothAddress &address);

ObjCStrongReference<IOBluetoothSDPUUID> iobluetooth_uuid(const QBluetoothUuid &uuid);
QBluetoothUuid qt_uuid(IOBluetoothSDPUUID *uuid);

QString qt_error_string(IOReturn errorCode);

} // namespace OSXBluetooth

Q_DECLARE_LOGGING_CATEGORY(QT_BT_OSX)

QT_END_NAMESPACE

#endif
