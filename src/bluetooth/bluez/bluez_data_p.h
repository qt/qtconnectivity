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

#ifndef BLUEZ_DATA_P_H
#define BLUEZ_DATA_P_H

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
#include <sys/socket.h>

#define BTPROTO_L2CAP   0
#define BTPROTO_RFCOMM  3

#define SOL_L2CAP   6
#define SOL_RFCOMM  18

#define RFCOMM_LM   0x03

#define RFCOMM_LM_AUTH      0x0002
#define RFCOMM_LM_ENCRYPT   0x0004
#define RFCOMM_LM_TRUSTED   0x0008
#define RFCOMM_LM_SECURE    0x0020

#define L2CAP_LM            0x03
#define L2CAP_LM_AUTH       0x0002
#define L2CAP_LM_ENCRYPT    0x0004
#define L2CAP_LM_TRUSTED    0x0008
#define L2CAP_LM_SECURE     0x0020

// Bluetooth address
typedef struct {
    quint8 b[6];
} __attribute__((packed)) bdaddr_t;

// L2CP socket
struct sockaddr_l2 {
    sa_family_t     l2_family;
    unsigned short  l2_psm;
    bdaddr_t        l2_bdaddr;
    unsigned short  l2_cid;
};

// RFCOMM socket
struct sockaddr_rc {
    sa_family_t rc_family;
    bdaddr_t    rc_bdaddr;
    quint8      rc_channel;
};

#endif // BLUEZ_DATA_P_H
