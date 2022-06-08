// Copyright (C) 2018 Governikus GmbH & Co. KG
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

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
