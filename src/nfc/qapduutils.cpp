/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtNfc module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qapduutils_p.h"
#include <QtCore/QtEndian>

QT_BEGIN_NAMESPACE

/*
    Utilities for handling smart card application protocol data units (APDU).

    The structure of APDUs is defined by ISO/IEC 7816-4 Organization, security
    and commands for interchange. The summary can be found on Wikipedia:

        https://en.wikipedia.org/wiki/Smart_card_application_protocol_data_unit
*/

/*
    Parses a response APDU from the raw data.

    If the data is too short to contain SW bytes, the returned responses SW
    is set to QResponseApdu::Empty.
*/
QResponseApdu::QResponseApdu(const QByteArray &response)
{
    if (response.size() < 2) {
        m_status = Empty;
        m_data = response;
    } else {
        const auto dataSize = response.size() - 2;
        m_status = qFromBigEndian(qFromUnaligned<uint16_t>(response.constData() + dataSize));
        m_data = response.left(dataSize);
    }
}

/*
    Builds a command APDU from components according to ISO/IEC 7816.
*/
QByteArray QCommandApdu::build(uint8_t cla, uint8_t ins, uint8_t p1, uint8_t p2,
                               QByteArrayView data, uint16_t ne)
{
    Q_ASSERT(data.size() <= 0xFFFF);

    QByteArray apdu;
    apdu.append(static_cast<char>(cla));
    apdu.append(static_cast<char>(ins));
    apdu.append(static_cast<char>(p1));
    apdu.append(static_cast<char>(p2));

    bool extendedLc = false;
    uint16_t nc = data.size();

    if (nc > 0) {
        if (nc < 256) {
            apdu.append(static_cast<char>(nc));
        } else {
            extendedLc = true;
            apdu.append('\0');
            apdu.append(static_cast<char>(nc >> 8));
            apdu.append(static_cast<char>(nc & 0xFF));
        }
        apdu.append(data);
    }

    if (ne) {
        if (ne < 256) {
            apdu.append(static_cast<char>(ne));
        } else if (ne == 256) {
            apdu.append(static_cast<char>('\0'));
        } else {
            if (!extendedLc)
                apdu.append('\0');
            apdu.append(static_cast<char>(ne >> 8));
            apdu.append(static_cast<char>(ne & 0xFF));
        }
    }

    return apdu;
}

QT_END_NAMESPACE
