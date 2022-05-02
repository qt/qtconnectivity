/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
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

#include "qlowenergyserviceprivate_p.h"

#include "qlowenergycontrollerbase_p.h"

QT_BEGIN_NAMESPACE

QLowEnergyServicePrivate::QLowEnergyServicePrivate(QObject *parent) : QObject(parent) { }

QLowEnergyServicePrivate::~QLowEnergyServicePrivate()
{
}

void QLowEnergyServicePrivate::setController(QLowEnergyControllerPrivate *control)
{
    controller = control;

    if (control)
        setState(QLowEnergyService::RemoteService);
    else
        setState(QLowEnergyService::InvalidService);
}

void QLowEnergyServicePrivate::setError(QLowEnergyService::ServiceError newError)
{
    lastError = newError;
    emit errorOccurred(newError);
}

void QLowEnergyServicePrivate::setState(QLowEnergyService::ServiceState newState)
{
    if (state == newState)
        return;

    state = newState;
    emit stateChanged(newState);
}

QT_END_NAMESPACE
