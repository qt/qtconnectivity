/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtNfc module of the Qt Toolkit.
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

#ifndef QNEARFIELDTARGET_EMULATOR_P_H
#define QNEARFIELDTARGET_EMULATOR_P_H

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

#include "qnearfieldtagtype1_p.h"
#include "qnearfieldtagtype2_p.h"
#include "targetemulator_p.h"

#include <QtCore/QMap>

QT_BEGIN_NAMESPACE

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

QT_END_NAMESPACE

#endif // QNEARFIELDTARGET_EMULATOR_P_H
