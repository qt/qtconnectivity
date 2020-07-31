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

#include "qnearfieldmanager.h"
#include "qnearfieldmanager_p.h"

#if defined(QT_SIMULATOR)
#include "qnearfieldmanager_simulator_p.h"
#elif defined(ANDROID_NFC)
#include "qnearfieldmanager_android_p.h"
#else
#include "qnearfieldmanager_generic_p.h"
#endif

#include <QtCore/QMetaType>
#include <QtCore/QMetaMethod>

QT_BEGIN_NAMESPACE

/*!
    \class QNearFieldManager
    \brief The QNearFieldManager class provides access to notifications for NFC events.

    \ingroup connectivity-nfc
    \inmodule QtNfc
    \since 5.2

    NFC Forum devices support two modes of communications.  The first mode, peer-to-peer
    communications, is used to communicate between two NFC Forum devices.  The second mode,
    master/slave communications, is used to communicate between an NFC Forum device and an NFC
    Forum Tag or Contactless Card. The targetDetected() signal is emitted when a target device
    enters communications range. Communications can be initiated from the slot connected to this
    signal.

    NFC Forum devices generally operate as the master in master/slave communications. Some devices
    are also capable of operating as the slave, so called Card Emulation mode. In this mode the
    local NFC device emulates a NFC Forum Tag or Contactless Card.

    Applications can connect to the targetDetected() and targetLost() signals to get notified when
    an NFC Forum Tag enters or leaves proximity. Before these signals are emitted target detection
    must be started with the startTargetDetection() function. Target detection can be stopped with
    the stopTargetDetection() function. When the target is no longer required the target should be
    deleted as other applications may be blocked from accessing the target.
*/

/*!
    \enum QNearFieldManager::AdapterState

    \since 5.12

    This enum describes the different states a NFC adapter can have.

    \value Offline      The nfc adapter is offline.
    \value TurningOn    The nfc adapter is turning on.
    \value Online       The nfc adapter is online.
    \value TurningOff   The nfc adapter is turning off.
*/

/*!
    \fn void QNearFieldManager::adapterStateChanged(AdapterState state)

    \since 5.12

    This signal is emitted whenever the \a state of the NFC adapter changed.

    \note Currently, this signal is only emitted on Android.
*/

/*!
    \fn void QNearFieldManager::targetDetected(QNearFieldTarget *target)

    This signal is emitted whenever a target is detected. The \a target parameter represents the
    detected target.

    This signal will be emitted for all detected targets.

    QNearFieldManager maintains ownership of \a target, however, it will not be destroyed until
    the QNearFieldManager destructor is called. Ownership may be transferred by calling
    setParent().

    Do not delete \a target from the slot connected to this signal, instead call deleteLater().

    \note that if \a target is deleted before it moves out of proximity the targetLost() signal
    will not be emitted.

    \sa targetLost()
*/

/*!
    \fn void QNearFieldManager::targetLost(QNearFieldTarget *target)

    This signal is emitted whenever a target moves out of proximity. The \a target parameter
    represents the lost target.

    Do not delete \a target from the slot connected to this signal, instead use deleteLater().

    \sa QNearFieldTarget::disconnected()
*/

/*!
    Constructs a new near field manager with \a parent.
*/
QNearFieldManager::QNearFieldManager(QObject *parent)
:   QObject(parent), d_ptr(new QNearFieldManagerPrivateImpl)
{
    qRegisterMetaType<AdapterState>();

    connect(d_ptr, &QNearFieldManagerPrivate::adapterStateChanged,
            this, &QNearFieldManager::adapterStateChanged);
    connect(d_ptr, &QNearFieldManagerPrivate::targetDetected,
            this, &QNearFieldManager::targetDetected);
    connect(d_ptr, &QNearFieldManagerPrivate::targetLost,
            this, &QNearFieldManager::targetLost);
}

/*!
    \internal

    Constructs a new near field manager with the specified \a backend and with \a parent.

    \note: This constructor is only enable for internal builds and is used for testing the
    simulator backend.
*/
QNearFieldManager::QNearFieldManager(QNearFieldManagerPrivate *backend, QObject *parent)
:   QObject(parent), d_ptr(backend)
{
    qRegisterMetaType<AdapterState>();

    connect(d_ptr, &QNearFieldManagerPrivate::adapterStateChanged,
            this, &QNearFieldManager::adapterStateChanged);
    connect(d_ptr, &QNearFieldManagerPrivate::targetDetected,
            this, &QNearFieldManager::targetDetected);
    connect(d_ptr, &QNearFieldManagerPrivate::targetLost,
            this, &QNearFieldManager::targetLost);
}

/*!
    Destroys the near field manager.
*/
QNearFieldManager::~QNearFieldManager()
{
    delete d_ptr;
}

/*!
    \since 6.0

    Returns \c true if the device has a NFC adapter and
    it is turned on; otherwise returns \c false.

    \sa isSupported()
*/
bool QNearFieldManager::isEnabled() const
{
    Q_D(const QNearFieldManager);

    return d->isEnabled();
}

/*!
    \since 5.12

    Returns \c true if the underlying device has a NFC adapter; otherwise returns \c false.

    \sa isEnabled()
*/
bool QNearFieldManager::isSupported() const
{
    Q_D(const QNearFieldManager);

    return d->isSupported();
}
/*!
    \fn bool QNearFieldManager::startTargetDetection()

    Starts detecting targets and returns true if target detection is
    successfully started; otherwise returns false. Causes the targetDetected() signal to be emitted
    when a target is within proximity.
    \sa stopTargetDetection()
*/
bool QNearFieldManager::startTargetDetection()
{
    Q_D(QNearFieldManager);
    return d->startTargetDetection();
}

/*!
    Stops detecting targets.  The targetDetected() signal will no longer be emitted until another
    call to startTargetDetection() is made.
*/
void QNearFieldManager::stopTargetDetection()
{
    Q_D(QNearFieldManager);

    d->stopTargetDetection();
}

QT_END_NAMESPACE
