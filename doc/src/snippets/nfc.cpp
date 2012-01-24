/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
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

#include <qndefrecord.h>
#include <qndefnfctextrecord.h>

#include <QtCore/QDebug>

QTNFC_USE_NAMESPACE

void snippet_recordConversion()
{
    QNdefRecord record;

    //! [Record conversion]
    if (record.isRecordType<QNdefNfcTextRecord>()) {
        QNdefNfcTextRecord textRecord(record);

        qDebug() << textRecord.text();
    }
    //! [Record conversion]
}

//! [Specialized class definition]
class ExampleComF : public QNdefRecord
{
public:
    Q_DECLARE_NDEF_RECORD(ExampleComF, QNdefRecord::ExternalRtd, "example.com:f",
                          QByteArray(sizeof(int), char(0)))

    int foo() const;
    void setFoo(int v);
};

Q_DECLARE_ISRECORDTYPE_FOR_NDEF_RECORD(ExampleComF, QNdefRecord::ExternalRtd, "example.com:f")
//! [Specialized class definition]
