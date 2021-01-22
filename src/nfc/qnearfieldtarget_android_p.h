/****************************************************************************
**
** Copyright (C) 2016 Centria research and development
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

#ifndef QNEARFIELDTARGET_ANDROID_P_H
#define QNEARFIELDTARGET_ANDROID_P_H

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

#include "android/androidjninfc_p.h"
#include "qnearfieldtarget.h"
#include "qnearfieldtarget_p.h"
#include "qndefmessage.h"
#include "qlist.h"
#include "qstringlist.h"
#include <QTimer>

#include <QtAndroidExtras/QAndroidJniObject>
#include <QtAndroidExtras/QAndroidJniEnvironment>

QT_BEGIN_NAMESPACE

class NearFieldTarget : public QNearFieldTarget
{
    Q_OBJECT
public:
    NearFieldTarget(QAndroidJniObject intent,
                    const QByteArray uid,
                    QObject *parent = 0);
    virtual ~NearFieldTarget();
    virtual QByteArray uid() const;
    virtual Type type() const;
    virtual AccessMethods accessMethods() const;
    bool keepConnection() const;
    bool setKeepConnection(bool isPersistent);
    bool disconnect();
    virtual bool hasNdefMessage();
    virtual RequestId readNdefMessages();
    int maxCommandLength() const;
    virtual RequestId sendCommand(const QByteArray &command);
    virtual RequestId sendCommands(const QList<QByteArray> &commands);
    virtual RequestId writeNdefMessages(const QList<QNdefMessage> &messages);
    void setIntent(QAndroidJniObject intent);

signals:
    void targetDestroyed(const QByteArray &tagId);
    void targetLost(QNearFieldTarget *target);
    void ndefMessageRead(const QNdefMessage &message, const QNearFieldTarget::RequestId &id);

protected slots:
    void checkIsTargetLost();

protected:
    void releaseIntent();
    void updateTechList();
    void updateType();
    Type getTagType() const;
    void setupTargetCheckTimer();
    void handleTargetLost();
    QAndroidJniObject getTagTechnology(const QString &tech) const;
    bool setTagTechnology(const QStringList &techList);
    bool connect();
    QByteArray jbyteArrayToQByteArray(const jbyteArray &byteArray) const;
    bool catchJavaExceptions(bool verbose = true) const;

protected:
    QAndroidJniObject m_intent;
    QByteArray m_uid;
    QStringList m_techList;
    Type m_type;
    QTimer *m_targetCheckTimer;
    QString m_tech;
    QAndroidJniObject m_tagTech;
    bool m_keepConnection;
};

QT_END_NAMESPACE

#endif // QNEARFIELDTARGET_ANDROID_P_H
