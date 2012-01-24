/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the QtNfc module of the Qt Toolkit.
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
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QNEARFIELDTAGTYPE3_H
#define QNEARFIELDTAGTYPE3_H

#include <qnearfieldtarget.h>

#include <QtCore/QList>
#include <QtCore/QMap>

QT_BEGIN_HEADER

QTNFC_BEGIN_NAMESPACE

class Q_NFC_EXPORT QNearFieldTagType3 : public QNearFieldTarget
{
    Q_OBJECT

public:
    explicit QNearFieldTagType3(QObject *parent = 0);

    Type type() const { return NfcTagType3; }

    quint16 systemCode();
    QList<quint16> services();
    int serviceMemorySize(quint16 serviceCode);

    virtual RequestId serviceData(quint16 serviceCode);
    virtual RequestId writeServiceData(quint16 serviceCode, const QByteArray &data);

    virtual RequestId check(const QMap<quint16, QList<quint16> > &serviceBlockList);
    virtual RequestId update(const QMap<quint16, QList<quint16> > &serviceBlockList,
                             const QByteArray &data);

protected:
    bool handleResponse(const QNearFieldTarget::RequestId &id, const QByteArray &response);
};

QTNFC_END_NAMESPACE

QT_END_HEADER

#endif // QNEARFIELDTAGTYPE3_H
