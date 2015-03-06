/****************************************************************************
**
** Copyright (C) 2015 BasysKom GmbH
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtNfc module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/


#include "androidmainnewintentlistener_p.h"
#include "qdebug.h"
#include <QtGui/QGuiApplication>
#include <QtAndroidExtras/QAndroidJniObject>

QT_BEGIN_ANDROIDNFC_NAMESPACE

MainNfcNewIntentListener::MainNfcNewIntentListener()
 : listeners(), listenersLock(), paused(true), receiving(false)
{
    QtAndroidPrivate::registerNewIntentListener(this);
    QtAndroidPrivate::registerResumePauseListener(this);
}

MainNfcNewIntentListener::~MainNfcNewIntentListener()
{
    QtAndroidPrivate::unregisterNewIntentListener(this);
    QtAndroidPrivate::unregisterResumePauseListener(this);
}

bool MainNfcNewIntentListener::handleNewIntent(JNIEnv */*env*/, jobject intent)
{
    listenersLock.lockForRead();
    foreach (AndroidNfc::AndroidNfcListenerInterface *listener, listeners) {
        listener->newIntent(QAndroidJniObject(intent));
    }
    listenersLock.unlock();
    return true;
}

bool MainNfcNewIntentListener::registerListener(AndroidNfcListenerInterface *listener)
{
    static bool firstListener = true;
    if (firstListener) {
        QAndroidJniObject intent = AndroidNfc::getStartIntent();
        if (intent.isValid()) {
            listener->newIntent(intent);
        }
        paused = static_cast<QGuiApplication*>(QGuiApplication::instance())->applicationState() != Qt::ApplicationActive;
    }
    firstListener = false;
    listenersLock.lockForWrite();
    if (!listeners.contains(listener))
        listeners.push_back(listener);
    listenersLock.unlock();
    updateReceiveState();
    return true;
}

bool MainNfcNewIntentListener::unregisterListener(AndroidNfcListenerInterface *listener)
{
    listenersLock.lockForWrite();
    listeners.removeOne(listener);
    listenersLock.unlock();
    updateReceiveState();
    return true;
}

void MainNfcNewIntentListener::handleResume()
{
    paused = false;
    updateReceiveState();
}

void MainNfcNewIntentListener::handlePause()
{
    paused = true;
    updateReceiveState();
}

void MainNfcNewIntentListener::updateReceiveState()
{
    if (paused) {
        // We were paused while receiving, so we stop receiving.
        if (receiving) {
            AndroidNfc::stopDiscovery();
            receiving = false;
        }
        return;
    }

    // We reach here, so we are not paused.
    listenersLock.lockForRead();
    // We have nfc listeners and do not receive. Switch on.
    if (listeners.count() && !receiving)
        receiving = AndroidNfc::startDiscovery();

    // we have no nfc listeners and do receive. Switch off.
    if (!listeners.count() && receiving) {
        AndroidNfc::stopDiscovery();
        receiving = false;
    }
    listenersLock.unlock();
}

QT_END_ANDROIDNFC_NAMESPACE
