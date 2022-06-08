// Copyright (C) 2016 BasysKom GmbH
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef ANDROIDMAINNEWINTENTLISTENER_P_H_
#define ANDROIDMAINNEWINTENTLISTENER_P_H_

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/private/qjnihelpers_p.h>

#include "qlist.h"
#include "qreadwritelock.h"
#include "androidjninfc_p.h"

QT_BEGIN_ANDROIDNFC_NAMESPACE

class MainNfcNewIntentListener : public QtAndroidPrivate::NewIntentListener, QtAndroidPrivate::ResumePauseListener
{
public:
    MainNfcNewIntentListener();
    ~MainNfcNewIntentListener();

    //QtAndroidPrivate::NewIntentListener
    bool handleNewIntent(JNIEnv *env, jobject intent);

    bool registerListener(AndroidNfcListenerInterface *listener);
    bool unregisterListener(AndroidNfcListenerInterface *listener);

    //QtAndroidPrivate::ResumePauseListener
    void handleResume();
    void handlePause();

private:
    void updateReceiveState();
protected:
    QList<AndroidNfc::AndroidNfcListenerInterface*> listeners;
    QReadWriteLock listenersLock;
    bool paused;
    bool receiving;
};

QT_END_ANDROIDNFC_NAMESPACE

#endif /* ANDROIDMAINNEWINTENTLISTENER_P_H_ */
