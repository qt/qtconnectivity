/***************************************************************************
**
** Copyright (C) 2013 BlackBerry Limited. All rights reserved.
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

#include "qlowenergyprocess_p.h"
#include <btapi/btdevice.h>
#include <errno.h>
#include "qlowenergyserviceinfo.h"
#include "qlowenergyserviceinfo_p.h"
#include <btapi/btgatt.h>
QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC(QLowEnergyProcess, processInstance)

void QLowEnergyProcess::handleEvent(const int event, const char *bt_address, const char *event_data)
{
    qDebug() << "[HANDLE Event] (event, address, event data): " << event << bt_address << event_data;
}

QLowEnergyProcess::QLowEnergyProcess()
{
    connected = false;
    if (bt_device_init( &(this->handleEvent) ) < 0)
        qDebug() << "[INIT] Init problem." << errno << strerror(errno);
    else
        connected = true;

}

/*!
    Destroys the QLowEnergyProcess object.
*/
QLowEnergyProcess::~QLowEnergyProcess()
{
    bt_device_deinit();
    bt_gatt_deinit();
    qDeleteAll(m_classPointers);
    m_classPointers.clear();
}

/*!
    Returns the instance of this clas. This class is a singleton class.
*/

QLowEnergyProcess *QLowEnergyProcess::instance()
{
    return processInstance();
}

bool QLowEnergyProcess::isConnected() const
{
    return connected;
}

void QLowEnergyProcess::addPointer(QLowEnergyServiceInfoPrivate* classPointer)
{
    m_classPointers.append(classPointer);
}
QT_END_NAMESPACE
