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
#ifndef LECMACCALCULATOR_H
#define LECMACCALCULATOR_H

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

QT_BEGIN_NAMESPACE

struct quint128;

class Q_AUTOTEST_EXPORT LeCmacCalculator
{
public:
    LeCmacCalculator();
    ~LeCmacCalculator();

    static QByteArray createFullMessage(const QByteArray &message, quint32 signCounter);

    quint64 calculateMac(const QByteArray &message, const quint128 &csrk) const;

    // Convenience function.
    bool verify(const QByteArray &message, const quint128 &csrk, quint64 expectedMac) const;

private:
    int m_baseSocket = -1;
};


QT_END_NAMESPACE

#endif // Header guard
