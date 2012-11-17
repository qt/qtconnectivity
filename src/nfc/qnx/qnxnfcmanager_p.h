/***************************************************************************
**
** Copyright (C) 2012 Research In Motion
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


#ifndef QNXNFCMANAGER_H
#define QNXNFCMANAGER_H

#include <QObject>
#include "nfc/nfc_types.h"
#include "nfc/nfc.h"
#include <QDebug>
#include <QSocketNotifier>
#include "../qndefmessage.h"
#include "../qndefrecord.h"
//#include <bb/system/InvokeManager>
//#include <bb/system/InvokeRequest>
//#include <bb/system/ApplicationStartupMode>
//#include "../qnearfieldtarget_blackberry_p.h"

#ifdef QNXNFC_DEBUG
#define qQNXNFCDebug qDebug
#else
#define qQNXNFCDebug QT_NO_QDEBUG_MACRO
#endif

QT_BEGIN_HEADER

QTNFC_BEGIN_NAMESPACE

class Q_DECL_EXPORT QNXNFCManager : public QObject
{
    Q_OBJECT
public:
    static QNXNFCManager *instance();
    void registerForNewInstance();
    void unregisterForInstance();
    nfc_target_t *getLastTarget();
    bool isAvailable();

private:
    QNXNFCManager();
    ~QNXNFCManager();

    static QNXNFCManager *m_instance;
    int m_instanceCount;

    int nfcFD;
    QSocketNotifier *nfcNotifier;

    QList<QNdefMessage> decodeTargetMessage(nfc_target_t *);

    QList<QPair<QObject*, QMetaMethod> > ndefMessageHandlers;

    //There can only be one target. The last detected one is saved here
    //currently we do not get notified when the target is disconnected. So the target might be invalid
    nfc_target_t *m_lastTarget;
    bool m_available;

public Q_SLOTS:
    void newNfcEvent(int fd);
    void nfcReadWriteEvent(nfc_event_t *nfcEvent);
    void startBTHandover();
    //TODO add a parameter to only detect a special target for now we are detecting all target types
    bool startTargetDetection();
    //void handleInvoke(const bb::system::InvokeRequest& request);

Q_SIGNALS:
    //void llcpEvent();
    //void ndefMessage(const QNdefMessage&);
    //void targetDetected(NearFieldTarget*, const QList<QNdefMessage>&);
    //Not sure if this is implementable
    void targetLost(); //Not available yet
    //void bluetoothHandover();
};

QTNFC_END_NAMESPACE

QT_END_HEADER

#endif

