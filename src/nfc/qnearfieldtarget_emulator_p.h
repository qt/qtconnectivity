/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Mobility Components.
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

#ifndef QNEARFIELDTARGET_EMULATOR_P_H
#define QNEARFIELDTARGET_EMULATOR_P_H

#include "qnearfieldtagtype1.h"
#include "qnearfieldtagtype2.h"
#include "targetemulator_p.h"

#include <QtCore/QMap>

QT_BEGIN_HEADER

QTNFC_BEGIN_NAMESPACE

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

QTNFC_END_NAMESPACE

QT_END_HEADER

#endif // QNEARFIELDTARGET_EMULATOR_P_H
