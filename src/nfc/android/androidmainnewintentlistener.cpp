// Copyright (C) 2016 BasysKom GmbH
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only


#include "androidmainnewintentlistener_p.h"
#include "android/androidjninfc_p.h"
#include "qdebug.h"
#include <QtGui/QGuiApplication>
#include <QtCore/QJniObject>

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
    // Only intents with a tag are relevant
    if (!AndroidNfc::getTag(intent).isValid())
        return false;

    listenersLock.lockForRead();
    for (AndroidNfc::AndroidNfcListenerInterface *listener : qAsConst(listeners)) {
        listener->newIntent(QJniObject(intent));
    }
    listenersLock.unlock();
    return true;
}

bool MainNfcNewIntentListener::registerListener(AndroidNfcListenerInterface *listener)
{
    static bool firstListener = true;
    if (firstListener) {
        QJniObject intent = AndroidNfc::getStartIntent();
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
    if (!listeners.isEmpty() && !receiving)
        receiving = AndroidNfc::startDiscovery();

    // we have no nfc listeners and do receive. Switch off.
    if (listeners.isEmpty() && receiving) {
        AndroidNfc::stopDiscovery();
        receiving = false;
    }
    listenersLock.unlock();
}

QT_END_ANDROIDNFC_NAMESPACE
