/***************************************************************************
 **
 ** Copyright (C) 2013 BlackBerry Limited. All rights reserved.
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

#ifndef QNXNFCSHAREMANAGER_P_H
#define QNXNFCSHAREMANAGER_P_H

#include "qnearfieldsharemanager.h"
#include <bb/system/NfcShareManager>
#include <QFileInfo>
#include <QNdefMessage>

QT_BEGIN_NAMESPACE

class Q_DECL_EXPORT QNXNFCShareManager : public QObject
{
    Q_OBJECT
public:
    static QNXNFCShareManager *instance();

    QNXNFCShareManager();
    ~QNXNFCShareManager();

    bool shareNdef(const QNdefMessage &message);
    bool shareFiles(const QList<QFileInfo> &files);
    void cancel();

    void setShareMode(bb::system::NfcShareMode::Type type);
    bb::system::NfcShareMode::Type shareMode() const;

    void connect(QObject *obj);
    void disconnect(QObject *obj);

    void reset();

    static QNearFieldShareManager::ShareError toShareError(bb::system::NfcShareError::Type nfcShareError);
    static QNearFieldShareManager::ShareModes toShareModes(bb::system::NfcShareMode::Type nfcShareMode);

private:
    bb::system::NfcShareManager *_manager;
    static const char *RECORD_NDEF;

Q_SIGNALS:
    void shareModeChanged(bb::system::NfcShareMode::Type);
    void error(bb::system::NfcShareError::Type);
    void finished(bb::system::NfcShareSuccess::Type);
    void targetAcquired();
    void targetCancelled();
};

QT_END_NAMESPACE

#endif /* QNXNFCSHAREMANAGER_P_H */
