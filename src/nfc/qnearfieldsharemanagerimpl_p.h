/***************************************************************************
 **
 ** Copyright (C) 2016 BlackBerry Limited. All rights reserved.
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
 ****************************************************************************/

#ifndef QNEARFIELDSHAREMANAGERIMPL_P_H_
#define QNEARFIELDSHAREMANAGERIMPL_P_H_

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

#include "qnearfieldsharemanager_p.h"

QT_BEGIN_NAMESPACE

class QNearFieldShareManagerPrivateImpl : public QNearFieldShareManagerPrivate
{
public:
    QNearFieldShareManagerPrivateImpl(QNearFieldShareManager* q);
    ~QNearFieldShareManagerPrivateImpl();

    static QNearFieldShareManager::ShareModes supportedShareModes();
};

QT_END_NAMESPACE

#endif /* QNEARFIELDSHAREMANAGERIMPL_P_H_ */
