/****************************************************************************
**
** Copyright (C) 2016 Centria research and development
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
#include "qnearfieldtarget_p.h"
#include "qndefmessage.h"
#include "qlist.h"
#include "qstringlist.h"
#include <QTimer>

#include <QtCore/QJniObject>
#include <QtCore/QJniEnvironment>

QT_BEGIN_NAMESPACE

class QNearFieldTargetPrivateImpl : public QNearFieldTargetPrivate
{
    Q_OBJECT
public:
    QNearFieldTargetPrivateImpl(QJniObject intent,
                                const QByteArray uid,
                                QObject *parent = nullptr);
    ~QNearFieldTargetPrivateImpl() override;

    QByteArray uid() const override;
    QNearFieldTarget::Type type() const override;
    QNearFieldTarget::AccessMethods accessMethods() const override;

    bool disconnect() override;

    bool hasNdefMessage() override;
    QNearFieldTarget::RequestId readNdefMessages() override;
    QNearFieldTarget::RequestId writeNdefMessages(const QList<QNdefMessage> &messages) override;

    int maxCommandLength() const override;
    QNearFieldTarget::RequestId sendCommand(const QByteArray &command) override;

    void setIntent(QJniObject intent);

signals:
    void targetDestroyed(const QByteArray &tagId);
    void targetLost(QNearFieldTargetPrivateImpl *target);
    void ndefMessageRead(const QNdefMessage &message, const QNearFieldTarget::RequestId &id);

protected slots:
    void checkIsTargetLost();

protected:
    void releaseIntent();
    void updateTechList();
    void updateType();
    QNearFieldTarget::Type getTagType() const;
    void setupTargetCheckTimer();
    void handleTargetLost();
    QJniObject getTagTechnology(const QString &tech) const;
    bool setTagTechnology(const QStringList &technologies);
    bool connect();
    bool setCommandTimeout(int timeout);
    QByteArray jbyteArrayToQByteArray(const jbyteArray &byteArray) const;

protected:
    QJniObject targetIntent;
    QByteArray targetUid;
    QTimer *targetCheckTimer;

    QString selectedTech;
    QStringList techList;
    QNearFieldTarget::Type tagType;
    QJniObject tagTech;
};

QT_END_NAMESPACE

#endif // QNEARFIELDTARGET_ANDROID_P_H
