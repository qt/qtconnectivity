// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef BTOBEXSESSION_P_H
#define BTOBEXSESSION_P_H

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

#include <QtCore/qvariant.h>
#include <QtCore/private/qglobal_p.h>

#include <Foundation/Foundation.h>

#include <IOBluetooth/IOBluetooth.h>

// TODO: all this code must be removed in Qt 6?

@class QT_MANGLE_NAMESPACE(DarwinBTOBEXSession);

QT_BEGIN_NAMESPACE

class QBluetoothAddress;
class QIODevice;
class QString;

namespace DarwinBluetooth
{

class OBEXSessionDelegate
{
public:
    typedef QT_MANGLE_NAMESPACE(DarwinBTOBEXSession) ObjCOBEXSession;

    virtual ~OBEXSessionDelegate();

    virtual void OBEXConnectError(OBEXError error, OBEXOpCode responseCode) = 0;
    virtual void OBEXConnectSuccess() = 0;

    virtual void OBEXAbortSuccess() = 0;

    virtual void OBEXPutDataSent(quint32 current, quint32 total) = 0;
    virtual void OBEXPutSuccess() = 0;
    virtual void OBEXPutError(OBEXError error, OBEXOpCode responseCode) = 0;
};

enum OBEXRequest {
    OBEXNoop,
    OBEXConnect,
    OBEXDisconnect,
    OBEXPut,
    OBEXGet,
    OBEXSetPath,
    OBEXAbort
};

} // namespace DarwinBluetooth

QT_END_NAMESPACE

// OBEX Session, it's a "single-shot" operation as our QBluetoothTransferReply is
// (it does not have an interface to re-send data or re-use the same transfer reply).
// It either succeeds or fails and tries to cleanup in any case.
@interface QT_MANGLE_NAMESPACE(DarwinBTOBEXSession) : NSObject

- (id)initWithDelegate:(QT_PREPEND_NAMESPACE(DarwinBluetooth::OBEXSessionDelegate) *)aDelegate
      remoteDevice:(const QBluetoothAddress &)deviceAddress channelID:(quint16)port;

- (void)dealloc;

// Below I have pairs: OBEX operation and its callback method.
- (OBEXError)OBEXConnect;
- (void)OBEXConnectHandler:(const OBEXSessionEvent*)event;

- (OBEXError)OBEXAbort;
- (void)OBEXAbortHandler:(const OBEXSessionEvent*)event;

- (OBEXError)OBEXPutFile:(QT_PREPEND_NAMESPACE(QIODevice) *)inputStream withName:(const QString &)name;
- (void)OBEXPutHandler:(const OBEXSessionEvent*)event;

// Aux. methods.
- (bool)isConnected;

// To be called from C++ destructors. OBEXSession is not
// valid anymore after this call (no more OBEX operations
// can be executed). It's an ABORT/DISCONNECT sequence.
// It also resets a delegate to null.
- (void)closeSession;
//
- (bool)hasActiveRequest;

@end

#endif
