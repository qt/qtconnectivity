/***************************************************************************
 **
 ** Copyright (C) 2016 BlackBerry Limited. All rights reserved.
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


#ifndef QNEARFIELDSHARETARGET_H
#define QNEARFIELDSHARETARGET_H

#include <QtCore/QObject>
#include <QtCore/QFileInfo>
#include <QtNfc/QNdefMessage>
#include <QtNfc/QNearFieldShareManager>

QT_BEGIN_NAMESPACE

class QNearFieldShareTargetPrivate;

class Q_NFC_EXPORT QNearFieldShareTarget : public QObject
{
    Q_OBJECT

public:
    ~QNearFieldShareTarget();

    QNearFieldShareManager::ShareModes shareModes() const;
    bool share(const QNdefMessage &message);
    bool share(const QList<QFileInfo> &files);
    void cancel();
    bool isShareInProgress() const;
    QNearFieldShareManager::ShareError shareError() const;

Q_SIGNALS:
    void error(QNearFieldShareManager::ShareError error);
    void shareFinished();

private:
    explicit QNearFieldShareTarget(QNearFieldShareManager::ShareModes modes, QObject *parent = nullptr);

    QNearFieldShareTargetPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QNearFieldShareTarget)
    Q_DISABLE_COPY(QNearFieldShareTarget)

    friend class QNearFieldShareManagerPrivateImpl;
    friend class QNearFieldShareTargetPrivateImpl;
};

QT_END_NAMESPACE

#endif /* QNEARFIELDSHARETARGET_H */
