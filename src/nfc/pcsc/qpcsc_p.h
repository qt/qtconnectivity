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

#ifndef QPCSC_P_H
#define QPCSC_P_H

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

#ifdef Q_OS_WIN
#    include <qt_windows.h>
#endif
#include <QtCore/QtGlobal>
#ifdef Q_OS_DARWIN
#    include <PCSC/winscard.h>
#    include <PCSC/wintypes.h>
#else
#    include <winscard.h>
#endif
#include <QtCore/QByteArray>
#include <QtCore/QString>

QT_BEGIN_NAMESPACE

namespace QPcsc {
struct RawCommandResult
{
    LONG ret = SCARD_E_READER_UNAVAILABLE;
    QByteArray response;

    bool isOk() const { return ret == SCARD_S_SUCCESS; }
};

QString errorMessage(LONG error);

} // namespace QPcsc

class QPcscSlotName : public
#ifdef Q_OS_WIN
                      QString
#else
                      QByteArray
#endif
{
public:
#ifdef Q_OS_WIN
    using CPtr = LPCWSTR;
    using Ptr = LPWSTR;
    explicit QPcscSlotName(CPtr p) : QString(reinterpret_cast<const QChar *>(p)) { }
#else
    using CPtr = LPCSTR;
    using Ptr = LPSTR;
    explicit QPcscSlotName(CPtr p) : QByteArray(p) { }
#endif

    CPtr ptr() const noexcept { return reinterpret_cast<CPtr>(constData()); }
    Ptr ptr() { return reinterpret_cast<Ptr>(data()); }

    static qsizetype nameSize(CPtr p);
};

QT_END_NAMESPACE

#endif // QPCSC_P_H
