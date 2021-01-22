/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
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

#include "osxbluetooth_p.h"

#include <QtCore/qvariant.h>
#include <QtCore/qglobal.h>

#include <Foundation/Foundation.h>

@class QT_MANGLE_NAMESPACE(OSXBTOBEXSession);

QT_BEGIN_NAMESPACE

class QBluetoothAddress;
class QIODevice;
class QString;

namespace OSXBluetooth
{

class OBEXSessionDelegate
{
public:
    typedef QT_MANGLE_NAMESPACE(OSXBTOBEXSession) ObjCOBEXSession;

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

}

QT_END_NAMESPACE

// OBEX Session, it's a "single-shot" operation as our QBluetoothTransferReply is
// (it does not have an interface to re-send data or re-use the same transfer reply).
// It either succeeds or fails and tries to cleanup in any case.
@interface QT_MANGLE_NAMESPACE(OSXBTOBEXSession) : NSObject

- (id)initWithDelegate:(QT_PREPEND_NAMESPACE(OSXBluetooth::OBEXSessionDelegate) *)aDelegate
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
