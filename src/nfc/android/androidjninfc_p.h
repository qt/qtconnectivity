/****************************************************************************
**
** Copyright (C) 2016 Centria research and development
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtNfc module of the Qt Toolkit.
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
