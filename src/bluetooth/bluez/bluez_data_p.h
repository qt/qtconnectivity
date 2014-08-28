/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia. For licensing terms and
** conditions see http://qt.digia.com/licensing. For further information
** use the contact form at http://qt.digia.com/contact-us.
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
** In addition, as a special exception, Digia gives you certain additional
** rights. These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
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
#include <QtBluetooth/QBluetoothUuid>

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

#define BDADDR_LE_PUBLIC    0x01

/* Byte order conversions */
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define htobs(d)  (d)
#define htobl(d)  (d)
#define htobll(d) (d)
#define btohs(d)  (d)
#define btohl(d)  (d)
#define btohll(d) (d)
#elif __BYTE_ORDER == __BIG_ENDIAN
#define htobs(d)  bswap_16(d)
#define htobl(d)  bswap_32(d)
#define htobll(d) bswap_64(d)
#define btohs(d)  bswap_16(d)
#define btohl(d)  bswap_32(d)
#define btohll(d) bswap_64(d)
#else
#error "Unknown byte order"
#endif

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
#if !defined(QT_BLUEZ_NO_BTLE)
    quint8          l2_bdaddr_type;
#endif
};

// RFCOMM socket
struct sockaddr_rc {
    sa_family_t rc_family;
    bdaddr_t    rc_bdaddr;
    quint8      rc_channel;
};

// Bt Low Energy related

#define bt_get_unaligned(ptr)           \
({                                      \
    struct __attribute__((packed)) {    \
        __typeof__(*(ptr)) __v;         \
    } *__p = (__typeof__(__p)) (ptr);   \
    __p->__v;                           \
})

#define bt_put_unaligned(val, ptr)      \
do {                                    \
    struct __attribute__((packed)) {    \
        __typeof__(*(ptr)) __v;         \
    } *__p = (__typeof__(__p)) (ptr);   \
    __p->__v = (val);                   \
} while (0)

#if __BYTE_ORDER == __LITTLE_ENDIAN

static inline void btoh128(const quint128 *src, quint128 *dst)
{
    memcpy(dst, src, sizeof(quint128));
}

static inline void ntoh128(const quint128 *src, quint128 *dst)
{
    int i;

    for (i = 0; i < 16; i++)
        dst->data[15 - i] = src->data[i];
}

static inline uint16_t bt_get_le16(const void *ptr)
{
    return bt_get_unaligned((const uint16_t *) ptr);
}
#elif __BYTE_ORDER == __BIG_ENDIAN
static inline uint16_t bt_get_le16(const void *ptr)
{
    return bswap_16(bt_get_unaligned((const uint16_t *) ptr));
}

static inline void btoh128(const quint128 *src, quint128 *dst)
{
    int i;

    for (i = 0; i < 16; i++)
        dst->data[15 - i] = src->data[i];
}

static inline void ntoh128(const quint128 *src, quint128 *dst)
{
    memcpy(dst, src, sizeof(quint128));
}
#else
#error "Unknown byte order"
#endif

#define hton128(x, y) ntoh128(x, y)

#endif // BLUEZ_DATA_P_H
