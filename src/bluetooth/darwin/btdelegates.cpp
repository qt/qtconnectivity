// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial

#include "btdelegates_p.h"

#if defined(Q_OS_MACOS)

QT_BEGIN_NAMESPACE

namespace DarwinBluetooth {

DeviceInquiryDelegate::~DeviceInquiryDelegate()
{
}

PairingDelegate::~PairingDelegate()
{
}

SDPInquiryDelegate::~SDPInquiryDelegate()
{
}

ChannelDelegate::~ChannelDelegate()
{
}

ConnectionMonitor::~ConnectionMonitor()
{
}

SocketListener::~SocketListener()
{
}

} // namespace DarwinBluetooth

QT_END_NAMESPACE

#endif
