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

#ifndef OSXBTSDPINQUIRY_H
#define OSXBTSDPINQUIRY_H

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

#include "qbluetoothaddress.h"
#include "qbluetoothuuid.h"
#include "osxbluetooth_p.h"

#include <QtCore/qglobal.h>
#include <QtCore/qlist.h>
#include <QtCore/qvector.h>

#include <Foundation/Foundation.h>

@class QT_MANGLE_NAMESPACE(OSXBTSDPInquiry);

QT_BEGIN_NAMESPACE

class QBluetoothServiceInfo;
class QVariant;

namespace DarwinBluetooth {

class SDPInquiryDelegate;

}

namespace OSXBluetooth {

void extract_service_record(IOBluetoothSDPServiceRecord *record, QBluetoothServiceInfo &serviceInfo);
QVariant extract_attribute_value(IOBluetoothSDPDataElement *dataElement);
QVector<QBluetoothUuid> extract_services_uuids(IOBluetoothDevice *device);

}

QT_END_NAMESPACE

@interface QT_MANGLE_NAMESPACE(OSXBTSDPInquiry) : NSObject

- (id)initWithDelegate:(QT_PREPEND_NAMESPACE(DarwinBluetooth::SDPInquiryDelegate) *)aDelegate;
- (void)dealloc;

- (IOReturn)performSDPQueryWithDevice:(const QBluetoothAddress &)address;
- (IOReturn)performSDPQueryWithDevice:(const QBluetoothAddress &)address
                              filters:(const QList<QBluetoothUuid> &)filters;

- (void)stopSDPQuery;

- (void)sdpQueryComplete:(IOBluetoothDevice *)aDevice status:(IOReturn)status;

@end

#endif
