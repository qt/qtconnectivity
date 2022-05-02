/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
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
******************************************************************************/

#ifndef QNEARFIELDMANAGER_H
#define QNEARFIELDMANAGER_H

#include <QtCore/QObject>
#include <QtNfc/qtnfcglobal.h>
#include <QtNfc/QNearFieldTarget>
#include <QtNfc/QNdefRecord>
#include <QtNfc/QNdefFilter>

QT_BEGIN_NAMESPACE

class QNearFieldManagerPrivate;
class Q_NFC_EXPORT QNearFieldManager : public QObject
{
    Q_OBJECT

    Q_DECLARE_PRIVATE(QNearFieldManager)

public:
    enum class AdapterState {
        Offline = 1,
        TurningOn = 2,
        Online = 3,
        TurningOff = 4
    };
    Q_ENUM(AdapterState)

    explicit QNearFieldManager(QObject *parent = nullptr);
    explicit QNearFieldManager(QNearFieldManagerPrivate *backend, QObject *parent = nullptr);
    ~QNearFieldManager();

    bool isEnabled() const;
    bool isSupported(QNearFieldTarget::AccessMethod accessMethod
                     = QNearFieldTarget::AnyAccess) const;

    bool startTargetDetection(QNearFieldTarget::AccessMethod accessMethod);
    void stopTargetDetection(const QString &errorMessage = QString());

    void setUserInformation(const QString &message);

Q_SIGNALS:
    void adapterStateChanged(QNearFieldManager::AdapterState state);
    void targetDetectionStopped();
    void targetDetected(QNearFieldTarget *target);
    void targetLost(QNearFieldTarget *target);

private:
    QNearFieldManagerPrivate *d_ptr;
};

QT_END_NAMESPACE

#endif // QNEARFIELDMANAGER_H
