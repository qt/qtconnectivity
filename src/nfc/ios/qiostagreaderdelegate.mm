/****************************************************************************
**
** Copyright (C) 2020 Governikus GmbH & Co. KG
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

#include "qiostagreaderdelegate_p.h"

#include "qnearfieldmanager_ios_p.h"

#import <CoreNFC/NFCError.h>
#import <CoreNFC/NFCTag.h>

QT_USE_NAMESPACE

@implementation QT_MANGLE_NAMESPACE(QIosTagReaderDelegate)

- (instancetype)initWithListener:(QNearFieldManagerPrivateImpl *)listener
{
    self = [super init];
    if (self) {
        self.listener = listener;
        self.sessionStoppedByApplication = false;
        self.message = nil;
        self.session = nil;
    }

    return self;
}

- (void)startSession
{
    if (self.sessionStoppedByApplication) {
        Q_EMIT self.listener->didInvalidateWithError(true);
        return;
    }

    if (self.session) {
        [self.session restartPolling];
        return;
    }

    self.session = [[[NFCTagReaderSession alloc] initWithPollingOption:NFCPollingISO14443 delegate:self queue:nil] autorelease];
    if (self.session) {
        if (self.message)
            self.session.alertMessage = self.message;
        [self.session beginSession];
    } else {
        Q_EMIT self.listener->didInvalidateWithError(true);
    }
}

- (void)stopSession:(QString)message
{
    if (self.session) {
        if (self.session.ready) {
            if (message.isNull())
                [self.session invalidateSession];
            else
               [self.session invalidateSessionWithErrorMessage:message.toNSString()];
           self.sessionStoppedByApplication = true;
        } else {
            self.session = nil;
        }
    }
}

- (void)alertMessage:(QString)message
{
    if (self.session && !self.sessionStoppedByApplication)
        self.session.alertMessage = message.toNSString();
    else
        self.message = message.toNSString();
}

- (void)tagReaderSessionDidBecomeActive:(NFCTagReaderSession*)session
{
    if (session != self.session)
        [session invalidateSession];
}

- (void)tagReaderSession:(NFCTagReaderSession*)session didInvalidateWithError:(NSError*)error
{
    if (session != self.session)
        return;

    self.session = nil;
    if (self.sessionStoppedByApplication) {
        self.sessionStoppedByApplication = false;
        return;
   }

    const bool doRestart =
        !(error.code == NFCReaderError::NFCReaderSessionInvalidationErrorUserCanceled
        || error.code == NFCReaderError::NFCReaderErrorUnsupportedFeature);
    Q_EMIT self.listener->didInvalidateWithError(doRestart);
}

- (void)tagReaderSession:(NFCTagReaderSession*)session didDetectTags:(NSArray<__kindof id<NFCTag>>*)tags
{
    if (session != self.session)
        return;

    bool foundTag = false;
    for (id<NFCTag> tag in tags) {
        if (tag.type == NFCTagTypeISO7816Compatible) {
            foundTag = true;
            [tag retain];
            Q_EMIT self.listener->tagDiscovered(tag);
        }
    }

    if (!foundTag) {
        [session restartPolling];
    }
}

@end
