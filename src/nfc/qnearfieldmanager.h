/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
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

#ifndef QNEARFIELDMANAGER_H
#define QNEARFIELDMANAGER_H

#include "qnfcglobal.h"

#include <QtCore/QObject>

#include <qnearfieldtarget.h>
#include <qndefrecord.h>
#include <qndeffilter.h>

QT_BEGIN_HEADER

QTNFC_BEGIN_NAMESPACE

class QNearFieldManagerPrivate;
class Q_NFC_EXPORT QNearFieldManager : public QObject
{
    Q_OBJECT

    Q_DECLARE_PRIVATE(QNearFieldManager)

public:
    enum TargetAccessMode {
        NoTargetAccess = 0x00,
        NdefReadTargetAccess = 0x01,
        NdefWriteTargetAccess = 0x02,
        TagTypeSpecificTargetAccess = 0x04
    };
    Q_DECLARE_FLAGS(TargetAccessModes, TargetAccessMode)

    explicit QNearFieldManager(QObject *parent = 0);
    explicit QNearFieldManager(QNearFieldManagerPrivate *backend, QObject *parent = 0);
    ~QNearFieldManager();

    bool isAvailable() const;

    void setTargetAccessModes(TargetAccessModes accessModes);
    TargetAccessModes targetAccessModes() const;

    bool startTargetDetection(const QList<QNearFieldTarget::Type> &targetTypes);
    bool startTargetDetection(QNearFieldTarget::Type targetType = QNearFieldTarget::AnyTarget);
    void stopTargetDetection();

    template<typename T>
    int registerNdefMessageHandler(QObject *object, const char *method);
    int registerNdefMessageHandler(QObject *object, const char *method);
    int registerNdefMessageHandler(QNdefRecord::TypeNameFormat typeNameFormat,
                                   const QByteArray &type,
                                   QObject *object, const char *method);
    int registerNdefMessageHandler(const QNdefFilter &filter,
                                   QObject *object, const char *method);

    bool unregisterNdefMessageHandler(int handlerId);

Q_SIGNALS:
    void targetDetected(QNearFieldTarget *target);
    void targetLost(QNearFieldTarget *target);
    void transactionDetected(const QByteArray &applicationIdentifier);

private:
    QNearFieldManagerPrivate *d_ptr;
};

template<typename T>
int QNearFieldManager::registerNdefMessageHandler(QObject *object, const char *method)
{
    T record;

    return registerNdefMessageHandler(record.userTypeNameFormat(), record.type(),
                                         object, method);
}

Q_DECLARE_OPERATORS_FOR_FLAGS(QNearFieldManager::TargetAccessModes)

QTNFC_END_NAMESPACE

QT_END_HEADER

#endif // QNEARFIELDMANAGER_H
