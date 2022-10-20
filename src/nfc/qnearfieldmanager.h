/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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
