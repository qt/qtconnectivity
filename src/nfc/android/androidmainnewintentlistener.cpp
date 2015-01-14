/****************************************************************************
**
** Copyright (C) 2015 BasysKom GmbH
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

#include "androidmainnewintentlistener_p.h"

QT_BEGIN_ANDROIDNFC_NAMESPACE

MainNfcNewIntentListener::MainNfcNewIntentListener()
 : listeners(), listenersLock()
{
    QtAndroidPrivate::registerNewIntentListener(this);
}

MainNfcNewIntentListener::~MainNfcNewIntentListener()
{
    QtAndroidPrivate::unregisterNewIntentListener(this);
}

bool MainNfcNewIntentListener::handleNewIntent(JNIEnv *env, jobject intent)
{
    AndroidNfc::AttachedJNIEnv aenv;
    listenersLock.lockForRead();
    foreach (AndroidNfc::AndroidNfcListenerInterface *listener, listeners) {
        // Making new global reference for each listener.
        // Listeners must release reference when it is not used anymore.
        jobject newIntentRef = env->NewGlobalRef(intent);
        listener->newIntent(newIntentRef);
    }
    listenersLock.unlock();
    return true;
}

bool MainNfcNewIntentListener::registerListener(AndroidNfcListenerInterface *listener)
{
    listenersLock.lockForWrite();
    if (!listeners.contains(listener))
        listeners.push_back(listener);
    listenersLock.unlock();
    return true;
}

bool MainNfcNewIntentListener::unregisterListener(AndroidNfcListenerInterface *listener)
{
    listenersLock.lockForWrite();
    listeners.removeOne(listener);
    listenersLock.unlock();
    return true;
}

QT_END_ANDROIDNFC_NAMESPACE
