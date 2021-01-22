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

#ifndef TARGETEMULATOR_P_H
#define TARGETEMULATOR_P_H

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

#include <QtCore/QtGlobal>
#include <QtCore/QByteArray>
#include <QtNfc/qtnfcglobal.h>

QT_FORWARD_DECLARE_CLASS(QSettings)

QT_BEGIN_NAMESPACE

class TagBase
{
public:
    TagBase();
    virtual ~TagBase();

    virtual void load(QSettings *settings) = 0;

    virtual QByteArray processCommand(const QByteArray &command) = 0;

    virtual QByteArray uid() const = 0;

    qint64 lastAccessTime() const { return lastAccess; }

protected:
    mutable qint64 lastAccess;
};

class NfcTagType1 : public TagBase
{
public:
    NfcTagType1();
    ~NfcTagType1();

    void load(QSettings *settings);

    QByteArray processCommand(const QByteArray &command);

    QByteArray uid() const;

private:
    quint8 readData(quint8 block, quint8 byte);

    quint8 hr0;
    quint8 hr1;

    QByteArray memory;
};

class NfcTagType2 : public TagBase
{
public:
    NfcTagType2();
    ~NfcTagType2();

    void load(QSettings *settings);

    QByteArray processCommand(const QByteArray &command);

    QByteArray uid() const;

private:
    QByteArray memory;
    quint8 currentSector;
    bool expectPacket2;
};

QT_END_NAMESPACE

#endif // TARGETEMULATOR_P_H
