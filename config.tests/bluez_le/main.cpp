// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>

int main()
{
    sockaddr_l2 addr;
    memset(&addr, 0, sizeof(addr));
    addr.l2_family = AF_BLUETOOTH;
    addr.l2_cid = 4;
    addr.l2_bdaddr_type = BDADDR_LE_PUBLIC;
}
