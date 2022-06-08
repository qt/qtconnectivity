// Copyright (C) 2016 Centria research and development
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#ifndef ANDROIDJNINFC_H
#define ANDROIDJNINFC_H

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

#include "qglobal.h"

#include <QtCore/QJniObject>

#define QT_USE_ANDROIDNFC_NAMESPACE using namespace ::AndroidNfc;
#define QT_BEGIN_ANDROIDNFC_NAMESPACE namespace AndroidNfc {
#define QT_END_ANDROIDNFC_NAMESPACE }

QT_BEGIN_ANDROIDNFC_NAMESPACE

class AndroidNfcListenerInterface
{
public:
    virtual ~AndroidNfcListenerInterface(){}
    virtual void newIntent(QJniObject intent) = 0;
};

bool startDiscovery();
bool stopDiscovery();
QJniObject getStartIntent();
bool isEnabled();
bool isSupported();
bool registerListener(AndroidNfcListenerInterface *listener);
bool unregisterListener(AndroidNfcListenerInterface *listener);
QJniObject getTag(const QJniObject &intent);

QT_END_ANDROIDNFC_NAMESPACE

#endif
