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

#ifndef QNEARFIELDSHAREMANAGER_H
#define QNEARFIELDSHAREMANAGER_H

#include <QtCore/QObject>
#include <QtNfc/qtnfcglobal.h>

QT_BEGIN_NAMESPACE

class QNearFieldShareManagerPrivate;
class QNearFieldShareTarget;

class Q_NFC_EXPORT QNearFieldShareManager : public QObject
{
    Q_OBJECT

public:
    explicit QNearFieldShareManager(QObject *parent = nullptr);
    ~QNearFieldShareManager();

    enum ShareError {
        NoError,
        UnknownError,
        InvalidShareContentError,
        ShareCanceledError,
        ShareInterruptedError,
        ShareRejectedError,
        UnsupportedShareModeError,
        ShareAlreadyInProgressError,
        SharePermissionDeniedError
    };
    Q_ENUM(ShareError)

    enum ShareMode {
        NoShare = 0x00,
        NdefShare = 0x01,
        FileShare = 0x02
    };
    Q_ENUM(ShareMode)
    Q_DECLARE_FLAGS(ShareModes, ShareMode)

public:
    static QNearFieldShareManager::ShareModes supportedShareModes();
    void setShareModes(ShareModes modes);
    QNearFieldShareManager::ShareModes shareModes() const;
    QNearFieldShareManager::ShareError shareError() const;

Q_SIGNALS:
    void targetDetected(QNearFieldShareTarget* shareTarget);
    void shareModesChanged(QNearFieldShareManager::ShareModes modes);
    void error(QNearFieldShareManager::ShareError error);

private:
    QScopedPointer<QNearFieldShareManagerPrivate> const d_ptr;
    Q_DECLARE_PRIVATE(QNearFieldShareManager)
    Q_DISABLE_COPY(QNearFieldShareManager)

    friend class QNearFieldShareManagerPrivateImpl;
    friend class QNearFieldShareTargetPrivateImpl;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QNearFieldShareManager::ShareModes)

QT_END_NAMESPACE

#endif /* QNEARFIELDSHAREMANAGER_H */
