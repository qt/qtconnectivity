/****************************************************************************
**
** Copyright (C) 2018 Governikus GmbH & Co. KG
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

package org.qtproject.qt.android.nfc;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.nfc.NfcAdapter;


public class QtNfcBroadcastReceiver extends BroadcastReceiver
{
    private Context qtContext;

    public QtNfcBroadcastReceiver(Context context)
    {
        qtContext = context;
        IntentFilter filter = new IntentFilter(NfcAdapter.ACTION_ADAPTER_STATE_CHANGED);
        qtContext.registerReceiver(this, filter);
    }

    public void unregisterReceiver()
    {
        qtContext.unregisterReceiver(this);
    }

    public void onReceive(Context context, Intent intent)
    {
        final int state = intent.getIntExtra(NfcAdapter.EXTRA_ADAPTER_STATE, NfcAdapter.STATE_OFF);
        jniOnReceive(state);
    }

    public native void jniOnReceive(int state);
}
