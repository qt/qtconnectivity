/***************************************************************************
**
** Copyright (C) 2013 Centria research and development
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtNfc module of the Qt Toolkit.
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

#ifndef QNEARFIELDTARGET_ANDROID_P_H
#define QNEARFIELDTARGET_ANDROID_P_H

#include "android/androidjninfc_p.h"
#include "qnearfieldtarget.h"
#include "qnearfieldtarget_p.h"
#include "qndefmessage.h"
#include "qlist.h"
#include "qstringlist.h"
#include <QTimer>


QT_BEGIN_NAMESPACE

class NearFieldTarget : public QNearFieldTarget
{
    Q_OBJECT
public:
    NearFieldTarget(jobject intent,
                    const QByteArray uid,
                    QObject *parent = 0);
    virtual ~NearFieldTarget();
    virtual QByteArray uid() const;
    virtual Type type() const;
    virtual AccessMethods accessMethods() const;
    virtual bool hasNdefMessage();
    virtual RequestId readNdefMessages();
    virtual RequestId sendCommand(const QByteArray &command);
    virtual RequestId sendCommands(const QList<QByteArray> &commands);
    virtual RequestId writeNdefMessages(const QList<QNdefMessage> &messages);
    void setIntent(jobject intent);

signals:
    void targetDestroyed(const QByteArray &tagId);
    void targetLost(QNearFieldTarget *target);

protected slots:
    void checkIsTargetLost();

protected:
    void releaseIntent();
    void updateTechList();
    void updateType();
    Type getTagType() const;
    void setupTargetCheckTimer();
    void handleTargetLost();
    jobject getTagTechnology(const QString &tech, JNIEnv *env) const;
    QByteArray jbyteArrayToQByteArray(const jbyteArray &byteArray, JNIEnv *env) const;
    bool catchJavaExceptions(JNIEnv *env) const;

protected:
    jobject m_intent;
    QByteArray m_uid;
    QStringList m_techList;
    Type m_type;
    static const QString NdefTechology;
    static const QString NdefFormatableTechnology;
    static const QString NfcATechnology;
    static const QString NfcBTechnology;
    static const QString NfcFTechnology;
    static const QString NfcVTechnology;
    static const QString MifareClassicTechnology;
    static const QString MifareUltralightTechnology;
    QTimer *m_targetCheckTimer;
};

QT_END_NAMESPACE

#endif // QNEARFIELDTARGET_ANDROID_P_H
