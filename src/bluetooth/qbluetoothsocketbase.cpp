// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qbluetoothsocketbase_p.h"

QT_BEGIN_NAMESPACE

QBluetoothSocketBasePrivate::QBluetoothSocketBasePrivate(QBluetoothSocket *qq)
    : QObject(), q_ptr{qq}
{

}

QBluetoothSocketBasePrivate::~QBluetoothSocketBasePrivate()
{

}

QT_END_NAMESPACE

#include "moc_qbluetoothsocketbase_p.cpp"
