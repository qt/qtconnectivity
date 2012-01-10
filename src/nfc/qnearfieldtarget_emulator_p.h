/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Mobility Components.
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
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QNEARFIELDTARGET_EMULATOR_P_H
#define QNEARFIELDTARGET_EMULATOR_P_H

#include "qnearfieldtagtype1.h"
#include "qnearfieldtagtype2.h"
#include "targetemulator_p.h"

#include <QtCore/QMap>

class TagType1 : public QNearFieldTagType1
{
    Q_OBJECT

public:
    TagType1(TagBase *tag, QObject *parent);
    ~TagType1();

    QByteArray uid() const;

    AccessMethods accessMethods() const;

    RequestId sendCommand(const QByteArray &command);
    bool waitForRequestCompleted(const RequestId &id, int msecs = 5000);

private:
    TagBase *m_tag;
};

class TagType2 : public QNearFieldTagType2
{
    Q_OBJECT

public:
    TagType2(TagBase *tag, QObject *parent);
    ~TagType2();

    QByteArray uid() const;

    AccessMethods accessMethods() const;

    RequestId sendCommand(const QByteArray &command);
    bool waitForRequestCompleted(const RequestId &id, int msecs = 5000);

private:
    TagBase *m_tag;
};

class TagActivator : public QObject
{
    Q_OBJECT

public:
    TagActivator();
    ~TagActivator();

    void initialize();
    void reset();

    static TagActivator *instance();

protected:
    void timerEvent(QTimerEvent *e);

signals:
    void tagActivated(TagBase *tag);
    void tagDeactivated(TagBase *tag);

private:
    QMap<TagBase *, bool>::Iterator m_current;
    int timerId;
};

#endif // QNEARFIELDTARGET_EMULATOR_P_H
