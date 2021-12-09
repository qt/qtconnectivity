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

#ifndef QAPDUUTILS_P_H
#define QAPDUUTILS_P_H

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

#include <QtCore/QByteArray>

QT_BEGIN_NAMESPACE

class QResponseApdu
{
public:
    static constexpr uint16_t Empty = 0x0000;
    static constexpr uint16_t Success = 0x9000;

    explicit QResponseApdu(const QByteArray &response = {});

    const QByteArray &data() const { return m_data; }
    uint16_t status() const { return m_status; }
    bool isOk() const { return m_status == Success; }

private:
    QByteArray m_data;
    uint16_t m_status;
};

namespace QCommandApdu {

// INS byte values for command APDUs
constexpr uint8_t Select = 0xA4;
constexpr uint8_t ReadBinary = 0xB0;
constexpr uint8_t GetData = 0xCA;
constexpr uint8_t UpdateBinary = 0xD6;

QByteArray build(uint8_t cla, uint8_t ins, uint8_t p1, uint8_t p2, QByteArrayView data,
                 uint16_t ne = 0);
};

QT_END_NAMESPACE

#endif // QAPDUUTILS_P_H
