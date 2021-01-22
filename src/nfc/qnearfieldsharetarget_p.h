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


#ifndef QNEARFIELDSHARETARGET_P_H
#define QNEARFIELDSHARETARGET_P_H

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

#include "qnearfieldsharetarget.h"

QT_BEGIN_NAMESPACE

class QNearFieldShareTargetPrivate : public QObject
{
    Q_OBJECT

public:
    QNearFieldShareTargetPrivate(QNearFieldShareManager::ShareModes modes, QNearFieldShareTarget *q)
    : QObject(q)
    {
        Q_UNUSED(modes)
    }

    ~QNearFieldShareTargetPrivate()
    {
    }

    virtual QNearFieldShareManager::ShareModes shareModes() const
    {
        return QNearFieldShareManager::NoShare;
    }

    virtual bool share(const QNdefMessage &message)
    {
        Q_UNUSED(message)
        return false;
    }

    virtual bool share(const QList<QFileInfo> &files)
    {
        Q_UNUSED(files)
        return false;
    }

    virtual void cancel()
    {
    }

    virtual bool isShareInProgress() const
    {
        return false;
    }

    virtual QNearFieldShareManager::ShareError shareError() const
    {
        return QNearFieldShareManager::NoError;
    }
};

QT_END_NAMESPACE

#endif /* QNEARFIELDSHARETARGET_P_H */
