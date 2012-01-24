/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the QtNfc module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qnearfieldllcpdevice_symbian_p.h"

QTNFC_BEGIN_NAMESPACE

QNearFieldLlcpDeviceSymbian::QNearFieldLlcpDeviceSymbian(RNfcServer& nfcServer, QObject *parent)
                                  : QNearFieldTarget(parent), mNfcServer(nfcServer)
{
    setAccessMethods(QNearFieldTarget::LlcpAccess);
}

QByteArray QNearFieldLlcpDeviceSymbian::uid() const
{
    return QByteArray();
}

QNearFieldTarget::Type QNearFieldLlcpDeviceSymbian::type() const
{
    return QNearFieldTarget::NfcForumDevice;
}

QNearFieldTarget::AccessMethods QNearFieldLlcpDeviceSymbian::accessMethods() const
{
    return QNearFieldTarget::LlcpAccess;
}

void QNearFieldLlcpDeviceSymbian::setAccessMethods(const QNearFieldTarget::AccessMethods& accessMethods)
{
}

#include "moc_qnearfieldllcpdevice_symbian_p.cpp"

QTNFC_END_NAMESPACE
